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
#include <memory>

#include "BookshelfStore.h"
#include "MappedInputManager.h"
#include "activities/util/ConfirmationActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"

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
  const auto entries = BOOKSHELF.getEntries();
  for (const auto& entry : entries) {
    if (Storage.exists(entry.path.c_str())) continue;

    const std::string candidate = "/read/" + fileNameFromPath(entry.path);
    if (Storage.exists(candidate.c_str())) {
      BOOKSHELF.updatePath(entry.path, candidate);
    }
  }
}

void BookshelfActivity::loadReadingState(ShelfBook& shelfBook) {
  if (BOOKSHELF.isFinished(shelfBook.book.path) || isFinishedPath(shelfBook.book.path)) {
    shelfBook.state = ShelfState::Finished;
    shelfBook.percentage = 100;
    shelfBook.banner = "100%";
    return;
  }

  if (!FsHelpers::hasEpubExtension(shelfBook.book.path)) {
    shelfBook.state = ShelfState::New;
    shelfBook.percentage = 0;
    shelfBook.banner = "NEW";
    return;
  }

  Epub epub(shelfBook.book.path, "/.crosspoint");
  epub.load(false, true);

  HalFile progressFile;
  if (!Storage.openFileForRead("SHELF", epub.getCachePath() + "/progress.bin", progressFile)) {
    shelfBook.state = ShelfState::New;
    shelfBook.percentage = 0;
    shelfBook.banner = "NEW";
    return;
  }

  uint8_t data[10]{};
  const int dataSize = progressFile.read(data, sizeof(data));
  if (dataSize != 4 && dataSize != 6 && dataSize != 10) {
    shelfBook.state = ShelfState::New;
    shelfBook.percentage = 0;
    shelfBook.banner = "NEW";
    return;
  }

  const int spineIndex = data[0] + (data[1] << 8);
  const int page = data[2] + (data[3] << 8);
  int totalPages = 0;
  if (dataSize >= 6) totalPages = data[4] + (data[5] << 8);

  // progress.bin stores zero-based page indexes. A chapter with N pages therefore
  // reaches its final page at N-1, so divide by N-1 rather than N. The old math
  // made even a true last page mathematically incapable of reaching 100%.
  float chapterProgress = 0.0f;
  if (totalPages > 1) {
    chapterProgress = std::clamp(static_cast<float>(page) / static_cast<float>(totalPages - 1), 0.0f, 1.0f);
  } else if (totalPages == 1) {
    chapterProgress = 1.0f;
  }

  // If the persisted position is the last page of the last spine item, treat it
  // as completed immediately. This matches the user's visible reading position
  // even before CrossPoint advances to the separate End-of-Book screen.
  const int spineCount = epub.getSpineItemsCount();
  const bool onFinalContentPage =
      spineCount > 0 && totalPages > 0 && spineIndex >= spineCount - 1 && page >= totalPages - 1;
  if (onFinalContentPage) {
    BOOKSHELF.markFinished(shelfBook.book.path);
    shelfBook.state = ShelfState::Finished;
    shelfBook.percentage = 100;
    shelfBook.banner = "100%";
    return;
  }

  float bookProgress = 0.0f;
  if (epub.getBookSize() > 0) {
    bookProgress = epub.calculateProgress(spineIndex, chapterProgress) * 100.0f;
  }

  shelfBook.state = ShelfState::Reading;
  shelfBook.percentage = std::clamp(static_cast<int>(std::lround(bookProgress)), 1, 99);
  shelfBook.banner = std::to_string(shelfBook.percentage) + "%";
}

void BookshelfActivity::ensureCoverThumb(ShelfBook& shelfBook) {
  if (shelfBook.book.coverBmpPath.empty()) return;

  shelfBook.coverThumbPath = UITheme::getCoverThumbPath(shelfBook.book.coverBmpPath, COVER_HEIGHT);
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

void BookshelfActivity::loadBooks() {
  books.clear();
  books.reserve(BOOKSHELF.getEntries().size());

  for (const auto& entry : BOOKSHELF.getEntries()) {
    ShelfBook shelfBook;
    shelfBook.book = RECENT_BOOKS.getDataFromBook(entry.path);
    shelfBook.addedAt = entry.addedAt;
    if (shelfBook.book.title.empty()) shelfBook.book.title = fallbackTitle(entry.path);
    loadReadingState(shelfBook);
    ensureCoverThumb(shelfBook);
    books.push_back(std::move(shelfBook));
  }

  std::stable_sort(books.begin(), books.end(), [](const ShelfBook& a, const ShelfBook& b) {
    const int aRank = stateRank(a.state);
    const int bRank = stateRank(b.state);
    if (aRank != bRank) return aRank < bRank;
    return a.addedAt > b.addedAt;
  });
}

void BookshelfActivity::onEnter() {
  UiListActivity::onEnter();

  repairFinishedPaths();
  BOOKSHELF.pruneMissing();
  loadBooks();
  nav.selected = books.empty() ? 0 : std::min(nav.selected, listCount() - 1);
  gridTopIndex = 0;
  requestUpdate();
}

void BookshelfActivity::onExit() {
  Activity::onExit();
  books.clear();
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
  promptRemoveBook(books[index].book.path, books[index].book.title);
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
    promptRemoveBook(books[nav.selected].book.path, books[nav.selected].book.title);
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

void BookshelfActivity::promptRemoveBook(const std::string& path, const std::string& title) {
  auto handler = [this, path](const ActivityResult& res) {
    if (res.isCancelled) return;

    if (BOOKSHELF.remove(path)) {
      loadBooks();
      if (books.empty()) {
        nav.selected = 0;
      } else if (nav.selected >= listCount()) {
        nav.selected = listCount() - 1;
      }
      ensureSelectionVisible();
      requestUpdate(true);
    }
  };

  startActivityForResult(std::make_unique<ConfirmationActivity>(renderer, mappedInput, "Remove from Bookshelf?", title),
                         std::move(handler));
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

  const int bannerY = rect.bottom() - STATUS_BANNER_HEIGHT;
  self->renderer.fillRect(rect.x + 1, bannerY, rect.width - 2, STATUS_BANNER_HEIGHT - 1);
  const int textWidth = self->renderer.getTextWidth(UI_10_FONT_ID, shelfBook.banner.c_str());
  const int textHeight = self->renderer.getLineHeight(UI_10_FONT_ID);
  const int textX = rect.x + (rect.width - textWidth) / 2;
  const int textY = bannerY + (STATUS_BANNER_HEIGHT - textHeight) / 2;
  self->renderer.drawText(UI_10_FONT_ID, textX, textY, shelfBook.banner.c_str(), false);
  return true;
}

void BookshelfActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  if (books.empty()) {
    screen.centeredText("No books on your shelf", screen.theme().bodyText);
    return;
  }

  const fui::Rect gridRect = screen.body();
  gridVisibleCells = fui::coverGridVisibleCells(gridRect, GRID_COLUMNS, GRID_ROW_HEIGHT, GRID_GAP);
  if (gridVisibleCells == 0) gridVisibleCells = GRID_COLUMNS;
  ensureSelectionVisible();

  std::vector<fui::CoverGridItem> items;
  items.reserve(books.size());
  for (size_t i = 0; i < books.size(); ++i) {
    fui::CoverGridItem item;
    item.title = books[i].book.title.c_str();
    item.actionValue = static_cast<int16_t>(i);
    items.push_back(item);
  }

  fui::CoverGridProps props;
  props.items = items.data();
  props.count = static_cast<uint16_t>(items.size());
  props.topIndex = gridTopIndex;
  props.selectedIndex = static_cast<int16_t>(nav.selected);
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch | fui::InputLongPress;
  props.columns = GRID_COLUMNS;
  props.coverSize = fui::Size{COVER_WIDTH, COVER_HEIGHT};
  props.rowHeight = GRID_ROW_HEIGHT;
  props.gap = GRID_GAP;
  props.rowGap = GRID_GAP;
  props.selectionIndicator = fui::CoverGridSelectionIndicator::CoverFrame;
  props.titleText = screen.theme().smallText;
  props.titleText.maxLines = 2;
  props.labelHeight = static_cast<int16_t>(screen.target().lineHeight(props.titleText.font) * 2);
  props.labelGap = 4;
  props.coverPainter = &BookshelfActivity::paintCover;
  props.coverPainterUserData = this;

  fui::coverGrid(screen.frame(), gridRect, props);
}

void BookshelfActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_OPEN), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
