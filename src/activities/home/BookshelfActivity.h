#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "RecentBooksStore.h"
#include "activities/UiListActivity.h"

class BookshelfActivity final : public UiListActivity {
 public:
  enum class ShelfState : uint8_t { Reading, New, Finished };

  explicit BookshelfActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;
  void onExit() override;

 private:
  struct ShelfBook {
    RecentBook book;
    ShelfState state = ShelfState::New;
    int percentage = 0;
    int recentRank = 1000;
    std::string coverThumbPath;
  };

  static constexpr uint8_t GRID_COLUMNS = 3;
  static constexpr int16_t COVER_WIDTH = 110;
  static constexpr int16_t COVER_HEIGHT = 166;
  static constexpr int16_t GRID_GAP = 8;
  static constexpr int16_t GRID_ROW_HEIGHT = 214;
  static constexpr uint16_t INITIAL_VISIBLE_CELLS = GRID_COLUMNS * 3;

  int listCount() const override { return static_cast<int>(books.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  void onRowLongPress(int index) override;
  bool handleButtons() override;
  bool handleCustomInput() override;
  void navigateButtons() override;
  const char* headerTitle() const override { return "Bookshelf"; }
  void drawFooter() override;

  std::vector<ShelfBook> books;
  bool longPressFired = false;
  uint16_t gridTopIndex = 0;
  uint16_t gridVisibleCells = INITIAL_VISIBLE_CELLS;

  void loadBooks();
  void repairFinishedPaths();
  void loadReadingState(ShelfBook& shelfBook);
  void ensureCoverThumb(ShelfBook& shelfBook);
  void prepareVisibleCovers();
  void selectIndex(int index);
  void ensureSelectionVisible();
  void showBookActions(int index);
  void markBookUnread(const std::string& path);
  void promptDeleteBook(const std::string& path, const std::string& title);
  void refreshAfterAction();

  static freeink::ui::CoverGridItem provideGridItem(uint16_t index, void* userData);
  static bool paintCover(freeink::ui::DrawTarget& target, freeink::ui::Rect rect,
                         const freeink::ui::CoverGridItem& item, uint16_t index, void* userData);
};
