#include "BookshelfActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <memory>

#include "BookshelfStore.h"
#include "MappedInputManager.h"
#include "activities/util/ConfirmationActivity.h"
#include "components/UITheme.h"
#include "components/UiAppHelpers.h"

namespace fui = freeink::ui;

namespace {
constexpr unsigned long LONG_PRESS_MS = 1000;

std::string fallbackTitle(const std::string& path) {
  const size_t slash = path.find_last_of('/');
  std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
  const size_t dot = name.find_last_of('.');
  if (dot != std::string::npos) name.resize(dot);
  return name;
}
}  // namespace

BookshelfActivity::BookshelfActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("Bookshelf", renderer, mappedInput, /*wantsTouchLongPress=*/true) {}

void BookshelfActivity::loadBooks() {
  books.clear();
  books.reserve(BOOKSHELF.getEntries().size());

  for (const auto& entry : BOOKSHELF.getEntries()) {
    RecentBook book = RECENT_BOOKS.getDataFromBook(entry.path);
    if (book.title.empty()) book.title = fallbackTitle(entry.path);
    books.push_back(std::move(book));
  }
}

void BookshelfActivity::onEnter() {
  UiListActivity::onEnter();

  BOOKSHELF.loadFromFile();
  BOOKSHELF.pruneMissing();
  loadBooks();
  requestUpdate();
}

void BookshelfActivity::onExit() {
  Activity::onExit();
  books.clear();
}

void BookshelfActivity::activateIndex(const int index) {
  app.clearTapFlash();
  if (index < 0 || index >= listCount()) return;
  LOG_DBG("SHELF", "Opening shelved book: %s", books[index].path.c_str());
  onSelectBook(books[index].path);
}

void BookshelfActivity::onRowLongPress(const int index) {
  app.clearTapFlash();
  if (index < 0 || index >= listCount()) return;
  promptRemoveBook(books[index].path, books[index].title);
}

bool BookshelfActivity::handleButtons() {
  if (longPressFired) {
    if (!mappedInput.isPressed(MappedInputManager::Button::Confirm)) longPressFired = false;
    return true;
  }

  if (!books.empty() && nav.selected < listCount() && mappedInput.isPressed(MappedInputManager::Button::Confirm) &&
      mappedInput.getHeldTime() >= LONG_PRESS_MS) {
    longPressFired = true;
    promptRemoveBook(books[nav.selected].path, books[nav.selected].title);
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
      nav.follow(listCount());
      requestUpdate(true);
    }
  };

  startActivityForResult(std::make_unique<ConfirmationActivity>(renderer, mappedInput, "Remove from Bookshelf?", title),
                         std::move(handler));
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

  std::vector<fui::ListItem> items;
  items.reserve(books.size());
  for (const auto& book : books) {
    fui::ListItem item;
    item.label = book.title.c_str();
    if (!book.author.empty()) item.subtitle = book.author.c_str();
    item.icon = listIconFor(UITheme::getFileIcon(book.path), 32);
    item.actionValue = static_cast<int16_t>(items.size());
    items.push_back(item);
  }

  fui::ListProps props;
  props.items = items.data();
  props.count = static_cast<uint16_t>(items.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch | fui::InputLongPress;
  fui::TextStyle label = screen.theme().smallText;
  label.bold = true;
  props.labelText = label;
  syncListViewport(screen, props);
  screen.list(props);
}

void BookshelfActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_OPEN), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
