#include "BookshelfActivity.h"

#include <Bitmap.h>
#include <Epub.h>
#include <FsHelpers.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <I18n.h>
#include <Xtc.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>

#include "BookshelfDiagnostics.h"
#include "BookshelfStore.h"
#include "MappedInputManager.h"
#include "activities/util/BookActionsActivity.h"
#include "activities/util/ConfirmationActivity.h"
#include "components/BookCoverGrid.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "util/BookCacheUtils.h"

namespace fui = freeink::ui;

namespace {
constexpr unsigned long LONG_PRESS_MS = 1000;
constexpr int16_t STATUS_BANNER_HEIGHT = 24;

std::string fallbackTitle(const std::string& path) {
  const size_t slash = path.find_last_of('/');
  std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
  const size_t dot = name.find_last_of('.');
  if (dot != std::string::npos) name.resize(dot);
  return name;
}

bool isFinishedPath(const std::string& path) { return path.rfind("/read/", 0) == 0; }

std::string fileNameFromPath(const std::string& path) {
  const size_t slash = path.find_last_of('/');
  return slash == std::string::npos ? path : path.substr(slash + 1);
}

int stateRank(const BookshelfActivity::ShelfState state) {
  switch (state) {
    case BookshelfActivity::ShelfState::Reading:
      return 0;
    case BookshelfActivity::ShelfState::New:
      return 1;
    case BookshelfActivity::ShelfState::Finished:
      return 2;
  }
  return 1;
}
}  // namespace

BookshelfActivity::BookshelfActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("Bookshelf", renderer, mappedInput, /*wantsTouchLongPress=*/true) {}

void BookshelfActivity::repairFinishedPaths() {
  const auto& entries = BOOKSHELF.getEntries();
  for (const auto& entry : entries) {
    if (Storage.exists(entry.path.c_str())) continue;

    const std::string oldPath = entry.path;
    const std::string candidate = "/read/" + fileNameFromPath(oldPath);
    if (Storage.exists(candidate.c_str())) {
      BOOKSHELF.updatePath(oldPath, candidate);
    }
  }
}

void BookshelfActivity::loadReadingState(ShelfBook& shelfBook) {
  const std::string& path = shelfBook.book.path;
  const bool explicitlyUnread = BOOKSHELF.isExplicitlyUnread(path);

  if (BOOKSHELF.isFinished(path)) {
    shelfBook.state = ShelfState::Finished;
    shelfBook.percentage = 100;
    return;
  }

  if (!FsHelpers::hasEpubExtension(path)) {
    if (isFinishedPath(path) && !explicitlyUnread) {
      BOOKSHELF.markFinished(path);
      shelfBook.state = ShelfState::Finished;
      shelfBook.percentage = 100;
    } else {
      shelfBook.state = ShelfState::New;
      shelfBook.percentage = 0;
    }
    return;
  }

  Epub epub(path, "/.crosspoint");
  epub.load(false, true);

  HalFile progressFile;
  if (!Storage.openFileForRead("SHELF", epub.getCachePath() + "/progress.bin", progressFile)) {
    if (isFinishedPath(path) && !explicitlyUnread) {
      BOOKSHELF.markFinished(path);
      shelfBook.state = ShelfState::Finished;
      shelfBook.percentage = 100;
    } else {
      shelfBook.state = ShelfState::New;
      shelfBook.percentage = 0;
    }
    return;
  }

  uint8_t data[10]{};
  const int dataSize = progressFile.read(data, sizeof(data));
  if (dataSize != 4 && dataSize != 6 && dataSize != 10) {
    shelfBook.state = ShelfState::New;
    shelfBook.percentage = 0;
    return;
  }

  if (explicitlyUnread) BOOKSHELF.markReading(path);

  const int spineIndex = data[0] + (data[1] << 8);
  const int page = data[2] + (data[3] << 8);
  int totalPages = 0;
  if (dataSize >= 6) totalPages = data[4] + (data[5] << 8);

  float chapterProgress = 0.0f;
  if (totalPages > 1) {
    chapterProgress = std::clamp(static_cast<float>(page) / static_cast<float>(totalPages - 1), 0.0f, 1.0f);
  } else if (totalPages == 1) {
    chapterProgress = 1.0f;
  }

  const int spineCount = epub.getSpineItemsCount();
  const bool onFinalContentPage =
      spineCount > 0 && totalPages > 0 && spineIndex >= spineCount - 1 && page >= totalPages - 1;
  if (onFinalContentPage) {
    BOOKSHELF.markFinished(path);
    shelfBook.state = ShelfState::Finished;
    shelfBook.percentage = 100;
    return;
  }

  float bookProgress = 0.0f;
  if (epub.getBookSize() > 0) {
    bookProgress = epub.calculateProgress(spineIndex, chapterProgress) * 100.0f;
  }

  shelfBook.state = ShelfState::Reading;
  shelfBook.percentage = std::clamp(static_cast<int>(std::lround(bookProgress)), 1, 99);
}

void BookshelfActivity::ensureCoverThumb(ShelfBook& shelfBook) {
  if (shelfBook.book.coverBmpPath.empty()) return;

  if (shelfBook.coverThumbPath.empty()) {
    shelfBook.coverThumbPath = UITheme::getCoverThumbPath(shelfBook.book.coverBmpPath, COVER_HEIGHT);
  }
  if (Storage.exists(shelfBook.coverThumbPath.c_str())) return;

  if (FsHelpers::hasEpubExtension(shelfBook.book.path)) {
    Epub epub(shelfBook.book.path, "/.crosspoint");
    epub.load(false, true);
    epub.generateThumbBmp(COVER_HEIGHT);
  } else if (FsHelpers::hasXtcExtension(shelfBook.book.path)) {
    Xtc xtc(shelfBook.book.path, "/.crosspoint");
    if (xtc.load()) xtc.generateThumbBmp(COVER_HEIGHT);
  }
}

void BookshelfActivity::prepareVisibleCovers() {
  if (books.empty() || gridVisibleCells == 0) return;
  const size_t first = std::min(static_cast<size_t>(gridTopIndex), books.size());
  const size_t last = std::min(first + static_cast<size_t>(gridVisibleCells), books.size());
  logBookshelfMemory("covers.begin", books.size());
  for (size_t i = first; i < last; ++i) ensureCoverThumb(books[i]);
  logBookshelfMemory("covers.end", books.size());
}

void BookshelfActivity::loadBooks() {
  books.clear();
  const auto& entries = BOOKSHELF.getEntries();
  books.reserve(entries.size());

  const auto& recents = RECENT_BOOKS.getBooks();

  for (auto entryIt = entries.rbegin(); entryIt != entries.rend(); ++entryIt) {
    const auto& entry = *entryIt;
    ShelfBook shelfBook;
    shelfBook.book = RECENT_BOOKS.getDataFromBook(entry.path);
    if (shelfBook.book.title.empty()) shelfBook.book.title = fallbackTitle(entry.path);

    for (size_t rank = 0; rank < recents.size(); ++rank) {
      if (recents[rank].path == entry.path) {
        shelfBook.recentRank = static_cast<int>(rank);
        break;
      }
    }

    loadReadingState(shelfBook);
    if (!shelfBook.book.coverBmpPath.empty()) {
      shelfBook.coverThumbPath = UITheme::getCoverThumbPath(shelfBook.book.coverBmpPath, COVER_HEIGHT);
    }
    books.push_back(std::move(shelfBook));
  }

  std::stable_sort(books.begin(), books.end(), [](const ShelfBook& a, const ShelfBook& b) {
    const int aRank = stateRank(a.state);
    const int bRank = stateRank(b.state);
    if (aRank != bRank) return aRank < bRank;
    if (a.state == ShelfState::Reading && a.recentRank != b.recentRank) return a.recentRank < b.recentRank;
    return false;
  });
}

void BookshelfActivity::onEnter() {
  UiListActivity::onEnter();
  logBookshelfMemory("enter.begin");

  repairFinishedPaths();
  BOOKSHELF.pruneMissing();
  loadBooks();
  logBookshelfMemory("load.end", books.size());

  nav.selected = books.empty() ? 0 : std::min(nav.selected, listCount() - 1);
  gridTopIndex = 0;
  gridVisibleCells = INITIAL_VISIBLE_CELLS;
  prepareVisibleCovers();
  logBookshelfMemory("enter.end", books.size());
  requestUpdate();
}

void BookshelfActivity::onExit() {
  Activity::onExit();
  logBookshelfMemory("exit.before_release", books.size());
  std::vector<ShelfBook>().swap(books);
  logBookshelfMemory("exit.after_release");
}

void BookshelfActivity::activateIndex(const int index) {
  app.clearTapFlash();
  if (index < 0 || index >= listCount()) return;
  LOG_DBG("SHELF", "Opening shelved book: %s", books[index].book.path.c_str());
  onSelectBook(books[index].book.path);
}

void BookshelfActivity::onRowLongPress(const int index) {
  app.clearTapFlash();
  if (index < 0 || index >= listCount()) return;
  showBookActions(index);
}

void BookshelfActivity::ensureSelectionVisible() {
  if (books.empty() || gridVisibleCells == 0) {
    gridTopIndex = 0;
    return;
  }

  gridTopIndex = fui::coverGridTopIndexFor(static_cast<uint16_t>(nav.selected), static_cast<uint16_t>(books.size()),
                                           GRID_COLUMNS, gridVisibleCells);
}

void BookshelfActivity::selectIndex(const int index) {
  if (books.empty()) return;
  nav.selected = std::clamp(index, 0, listCount() - 1);
  ensureSelectionVisible();
  prepareVisibleCovers();
  requestUpdate();
}

bool BookshelfActivity::handleCustomInput() {
  const auto swipe = mappedInput.wasSwipe();
  if (swipe != MappedInputManager::SwipeDir::Up && swipe != MappedInputManager::SwipeDir::Down) return false;
  if (books.empty() || gridVisibleCells == 0) return true;

  const int count = listCount();
  const int page = static_cast<int>(gridVisibleCells);
  const int maxTop = ((count - 1) / page) * page;
  if (swipe == MappedInputManager::SwipeDir::Up) {
    gridTopIndex = static_cast<uint16_t>(std::min(static_cast<int>(gridTopIndex) + page, maxTop));
  } else {
    gridTopIndex = static_cast<uint16_t>(std::max(static_cast<int>(gridTopIndex) - page, 0));
  }
  nav.selected = std::min(static_cast<int>(gridTopIndex), count - 1);
  prepareVisibleCovers();
  requestUpdate();
  return true;
}

bool BookshelfActivity::handleButtons() {
  if (longPressFired) {
    if (!mappedInput.isPressed(MappedInputManager::Button::Confirm)) longPressFired = false;
    return true;
  }

  if (!books.empty() && nav.selected < listCount() && mappedInput.isPressed(MappedInputManager::Button::Confirm) &&
      mappedInput.getHeldTime() >= LONG_PRESS_MS) {
    longPressFired = true;
    showBookActions(nav.selected);
    return true;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    if (!books.empty() && nav.selected < listCount()) {
      activateIndex(nav.selected);
      return true;
    }
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome();
    return true;
  }

  return false;
}

void BookshelfActivity::navigateButtons() {
  const int count = listCount();
  if (count <= 0) return;

  buttonNavigator.onNextRelease([this, count] { selectIndex(ButtonNavigator::nextIndex(nav.selected, count)); });
  buttonNavigator.onPreviousRelease(
      [this, count] { selectIndex(ButtonNavigator::previousIndex(nav.selected, count)); });
  buttonNavigator.onNextContinuous(
      [this, count] { selectIndex(std::min(nav.selected + static_cast<int>(gridVisibleCells), count - 1)); });
  buttonNavigator.onPreviousContinuous(
      [this] { selectIndex(std::max(nav.selected - static_cast<int>(gridVisibleCells), 0)); });
}

void BookshelfActivity::refreshAfterAction() {
  loadBooks();
  if (books.empty()) {
    nav.selected = 0;
    gridTopIndex = 0;
  } else if (nav.selected >= listCount()) {
    nav.selected = listCount() - 1;
    ensureSelectionVisible();
  } else {
    ensureSelectionVisible();
  }
  prepareVisibleCovers();
  requestUpdate(true);
}

void BookshelfActivity::markBookUnread(const std::string& path) {
  if (FsHelpers::hasEpubExtension(path)) {
    Epub epub(path, "/.crosspoint");
    epub.load(false, true);
    const std::string progressPath = epub.getCachePath() + "/progress.bin";
    if (Storage.exists(progressPath.c_str())) Storage.remove(progressPath.c_str());
  }
  BOOKSHELF.markUnread(path);
}

void BookshelfActivity::promptDeleteBook(const std::string& path, const std::string& title) {
  auto handler = [this, path](const ActivityResult& res) {
    if (res.isCancelled) return;

    if (Storage.remove(path.c_str())) {
      BOOKSHELF.remove(path);
      RECENT_BOOKS.removeByPath(path);
      clearBookCache(path);
      refreshAfterAction();
    } else {
      LOG_ERR("SHELF", "Failed to delete book from library: %s", path.c_str());
    }
  };

  startActivityForResult(std::make_unique<ConfirmationActivity>(renderer, mappedInput, "Delete from Library?", title),
                         std::move(handler));
}

void BookshelfActivity::showBookActions(const int index) {
  if (index < 0 || index >= listCount()) return;

  const std::string path = books[index].book.path;
  const std::string title = books[index].book.title;
  const bool finished = books[index].state == ShelfState::Finished;

  auto handler = [this, path, title, finished](const ActivityResult& res) {
    if (res.isCancelled) return;
    const auto* menu = std::get_if<MenuResult>(&res.data);
    if (!menu) return;

    switch (menu->action) {
      case BookActionsActivity::OPEN:
        onSelectBook(path);
        break;
      case BookActionsActivity::TOGGLE_FINISHED:
        if (finished) {
          markBookUnread(path);
        } else {
          BOOKSHELF.markFinished(path);
        }
        refreshAfterAction();
        break;
      case BookActionsActivity::TOGGLE_BOOKSHELF:
        BOOKSHELF.remove(path);
        refreshAfterAction();
        break;
      case BookActionsActivity::DELETE_FROM_LIBRARY:
        promptDeleteBook(path, title);
        break;
      default:
        break;
    }
  };

  startActivityForResult(std::make_unique<BookActionsActivity>(renderer, mappedInput, true, finished, true),
                         std::move(handler));
}

fui::CoverGridItem BookshelfActivity::provideGridItem(const uint16_t index, void* userData) {
  auto* self = static_cast<BookshelfActivity*>(userData);
  if (!self || index >= self->books.size()) return {};

  fui::CoverGridItem item;
  item.title = self->books[index].book.title.c_str();
  item.actionValue = static_cast<int16_t>(index);
  return item;
}

bool BookshelfActivity::paintCover(fui::DrawTarget& target, const fui::Rect rect, const fui::CoverGridItem& item,
                                   const uint16_t index, void* userData) {
  (void)target;
  (void)item;
  auto* self = static_cast<BookshelfActivity*>(userData);
  if (!self || index >= self->books.size()) return false;

  const ShelfBook& shelfBook = self->books[index];
  bool coverDrawn = false;
  if (!shelfBook.coverThumbPath.empty()) {
    HalFile file;
    if (Storage.openFileForRead("SHELF", shelfBook.coverThumbPath, file)) {
      Bitmap bitmap(file);
      if (bitmap.parseHeaders() == BmpReaderError::Ok) {
        self->renderer.drawBitmap(bitmap, rect.x, rect.y, rect.width, rect.height);
        coverDrawn = true;
      }
    }
  }

  if (!coverDrawn) {
    self->renderer.fillRect(rect.x, rect.y, rect.width, rect.height, false);
  }
  self->renderer.drawRect(rect.x, rect.y, rect.width, rect.height);

  char banner[8]{};
  if (shelfBook.state == ShelfState::New) {
    snprintf(banner, sizeof(banner), "NEW");
  } else if (shelfBook.state == ShelfState::Finished) {
    snprintf(banner, sizeof(banner), "100%%");
  } else {
    snprintf(banner, sizeof(banner), "%d%%", shelfBook.percentage);
  }

  const int bannerY = rect.bottom() - STATUS_BANNER_HEIGHT;
  self->renderer.fillRect(rect.x + 1, bannerY, rect.width - 2, STATUS_BANNER_HEIGHT - 1);
  const int textWidth = self->renderer.getTextWidth(UI_10_FONT_ID, banner);
  const int textHeight = self->renderer.getLineHeight(UI_10_FONT_ID);
  const int textX = rect.x + (rect.width - textWidth) / 2;
  const int textY = bannerY + (STATUS_BANNER_HEIGHT - textHeight) / 2;
  self->renderer.drawText(UI_10_FONT_ID, textX, textY, banner, false);
  return true;
}

void BookshelfActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  if (books.empty()) {
    screen.centeredText("Your Bookshelf is empty", screen.theme().bodyText);
    screen.spacer(static_cast<int16_t>(metrics.verticalSpacing * 2));
    screen.centeredText("Long-press a book in Browse", screen.theme().smallText);
    screen.centeredText("and choose Add to Bookshelf", screen.theme().smallText);
    return;
  }

  const fui::Rect gridRect = screen.body();
  const BookCoverGrid::Layout layout{GRID_COLUMNS, COVER_WIDTH, COVER_HEIGHT, GRID_ROW_HEIGHT, GRID_GAP};
  gridVisibleCells = BookCoverGrid::visibleCells(gridRect, layout);
  if (gridVisibleCells == 0) gridVisibleCells = GRID_COLUMNS;
  ensureSelectionVisible();

  auto props = BookCoverGrid::makeProps(screen, layout, static_cast<uint16_t>(books.size()), gridTopIndex,
                                        static_cast<int16_t>(nav.selected), &BookshelfActivity::provideGridItem, this,
                                        &BookshelfActivity::paintCover, this, ACTION_ROW,
                                        fui::InputTouch | fui::InputLongPress);
  fui::coverGrid(screen.frame(), gridRect, props);
}

void BookshelfActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_OPEN), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
