#include "HomeActivity.h"

#include <BoardConfig.h>
#include <GfxRenderer.h>
#include <I18n.h>

#include <memory>
#include <vector>

#include "BookshelfActivity.h"
#include "CrossPointState.h"
#include "MappedInputManager.h"
#include "components/UITheme.h"

namespace {
bool firstHomeEntryThisBoot = true;
}

void HomeActivity::onEnter() {
  Activity::onEnter();

  selectorIndex = initialMenuItem == HomeMenuItem::NONE ? 0 : menuItemToIndex(initialMenuItem);

  // CrossPoint already resumes the reader directly when sleep happened while reading.
  // If boot instead lands on Home on the X4 Pro, make the curated Bookshelf the normal
  // reading-first destination. The one-shot guard preserves Home as an explicit hub for
  // the rest of the session.
  if (firstHomeEntryThisBoot) {
    firstHomeEntryThisBoot = false;
    if (BoardConfig::isX4Pro() && !APP_STATE.lastSleepFromReader) {
      onBookshelfOpen();
      return;
    }
  }

  requestUpdate();
}

void HomeActivity::loop() {
  constexpr int menuCount = getMenuItemCount();
  const auto& metrics = UITheme::getInstance().getMetrics();

  auto activateSelection = [this] {
    switch (indexToMenuItem(selectorIndex)) {
      case HomeMenuItem::BOOKSHELF:
        onBookshelfOpen();
        break;
      case HomeMenuItem::FILE_BROWSER:
        onFileBrowserOpen();
        break;
      case HomeMenuItem::FILE_TRANSFER:
        onFileTransferOpen();
        break;
      case HomeMenuItem::SETTINGS_MENU:
        onSettingsOpen();
        break;
      default:
        break;
    }
  };

  buttonNavigator.onNext([this] {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, getMenuItemCount());
    requestUpdate();
  });

  buttonNavigator.onPrevious([this] {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, getMenuItemCount());
    requestUpdate();
  });

  const auto swipe = mappedInput.wasSwipe();
  if (swipe == MappedInputManager::SwipeDir::Up) {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, menuCount);
    requestUpdate();
    return;
  }
  if (swipe == MappedInputManager::SwipeDir::Down) {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, menuCount);
    requestUpdate();
    return;
  }

  const int menuTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  int menuRow = -1;
  const int menuRowHeight = GUI.getMenuRowHeight(renderer);
  const auto menuTouch = mappedInput.rowTouch(menuRow, menuTop, menuRowHeight + metrics.menuSpacing, menuCount, 0,
                                              INT32_MAX, menuRowHeight);
  if (menuTouch != MappedInputManager::RowTouch::None) {
    if (menuTouch == MappedInputManager::RowTouch::Down) {
      if (selectorIndex != menuRow) {
        selectorIndex = menuRow;
        requestUpdate();
      }
    } else {
      selectorIndex = menuRow;
      activateSelection();
    }
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    activateSelection();
  }
}

void HomeActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  renderer.clearScreen();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, nullptr);

  std::vector<const char*> menuItems = {"Bookshelf", "Library", tr(STR_FILE_TRANSFER), tr(STR_SETTINGS_TITLE)};
  std::vector<UIIcon> menuIcons = {Library, Folder, Transfer, Settings};

  const int menuTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  GUI.drawButtonMenu(renderer,
                     Rect{0, menuTop, pageWidth,
                          pageHeight - menuTop - metrics.buttonHintsHeight - metrics.verticalSpacing},
                     static_cast<int>(menuItems.size()), selectorIndex,
                     [&menuItems](int index) { return std::string(menuItems[index]); },
                     [&menuIcons](int index) { return menuIcons[index]; });

  const auto labels = mappedInput.mapLabels("", tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

void HomeActivity::onFileBrowserOpen() { activityManager.goToFileBrowser(); }

void HomeActivity::onBookshelfOpen() {
  activityManager.replaceActivity(std::make_unique<BookshelfActivity>(renderer, mappedInput));
}

void HomeActivity::onSettingsOpen() { activityManager.goToSettings(); }

void HomeActivity::onFileTransferOpen() { activityManager.goToFileTransfer(); }
