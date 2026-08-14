#pragma once

#include "./FileBrowserActivity.h"
#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class HomeActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  int selectorIndex = 0;
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

  void onFileBrowserOpen();
  void onBookshelfOpen();
  void onSettingsOpen();
  void onFileTransferOpen();

  static constexpr int getMenuItemCount() { return 4; }

 public:
  explicit HomeActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                        HomeMenuItem initialMenuItemValue = HomeMenuItem::NONE)
      : Activity("Home", renderer, mappedInput), initialMenuItem(initialMenuItemValue) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool isHomeActivity() const override { return true; }
};
