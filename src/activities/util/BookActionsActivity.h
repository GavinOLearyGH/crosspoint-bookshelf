#pragma once

#include "activities/UiListActivity.h"

class BookActionsActivity final : public UiListActivity {
 public:
  enum Action { OPEN = 0, TOGGLE_BOOKSHELF = 1, DELETE_FROM_DEVICE = 2 };

  BookActionsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, bool onBookshelf);

 private:
  int listCount() const override { return 3; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  bool handleButtons() override;
  const char* headerTitle() const override { return "Book Actions"; }

  bool onBookshelf = false;
};
