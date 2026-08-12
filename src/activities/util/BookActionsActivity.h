#pragma once

#include "activities/UiListActivity.h"

class BookActionsActivity final : public UiListActivity {
 public:
  enum Action {
    OPEN = 0,
    TOGGLE_FINISHED = 1,
    TOGGLE_BOOKSHELF = 2,
    DELETE_FROM_LIBRARY = 3,
    DELETE_FROM_DEVICE = DELETE_FROM_LIBRARY
  };

  BookActionsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, bool onBookshelf, bool finished = false,
                      bool shelfContext = false);

 private:
  int listCount() const override { return shelfContext ? 4 : 3; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  bool handleButtons() override;
  const char* headerTitle() const override { return "Book Actions"; }

  bool onBookshelf = false;
  bool finished = false;
  bool shelfContext = false;
};
