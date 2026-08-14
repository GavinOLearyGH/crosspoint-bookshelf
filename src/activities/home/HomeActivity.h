#pragma once
#include <functional>
#include <vector>

#include "./FileBrowserActivity.h"
#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

struct RecentBook;
struct Rect;

class HomeActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  int selectorIndex = 0;
  bool recentsLoading = false;
  bool recentsLoaded = false;
  bool firstRenderDone = false;
  bool coverRendered = false;
  bool coverBufferStored = false;
  bool backPressSeen = false;
  uint8_t* coverBuffer = nullptr;
  size_t coverBufferSize = 0;
  int coverRectX = 0;
  int coverRectY = 0;
  int coverRectW = 0;
  int coverRectH = 0;
  std::vector<RecentBook> recentBooks;
  const HomeMenuItem initialMenuItem;

  static int menuItemToIndex(HomeMenuItem item) {
    switch (item) {
      case HomeMenuItem::BOOKSHELF:
        return 0;
      case HomeMenuItem::FILE_BROWSER:
        return 1;
      case HomeMenuItem::FILE_TRANSFER:
        return 2;
      case HomeMenuItem::SETTINGS_MENU:
        return 3;
      default:
        return 0;
    }
  }

  static HomeMenuItem indexToMenuItem(int idx) {
    switch (idx) {
      case 0:
        return HomeMenuItem::BOOKSHELF;
      case 1:
        return HomeMenuItem::FILE_BROWSER;
      case 2:
        return HomeMenuItem::FILE_TRANSFER;
      case 3:
        return HomeMenuItem::SETTINGS_MENU;
      default:
        return HomeMenuItem::NONE;
    }
  }

  void onSelectBook(const std::string& path);
  void onFileBrowserOpen();
  void onBookshelfOpen();
  void onSettingsOpen();
  void onFileTransferOpen();

  int getMenuItemCount() const { return static_cast<int>(recentBooks.size()) + 4; }
  bool storeCoverBuffer();
  bool restoreCoverBuffer();
  void freeCoverBuffer();
  void loadRecentBooks(int maxBooks);
  void loadRecentCovers(int coverHeight);

 public:
  explicit HomeActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                        HomeMenuItem initialMenuItemValue = HomeMenuItem::NONE)
      : Activity("Home", renderer, mappedInput), initialMenuItem(initialMenuItemValue) {}
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool isHomeActivity() const override { return true; }
};
