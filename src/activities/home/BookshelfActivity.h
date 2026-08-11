#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "RecentBooksStore.h"
#include "activities/UiListActivity.h"

class BookshelfActivity final : public UiListActivity {
 public:
  explicit BookshelfActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;
  void onExit() override;

 private:
  enum class ShelfState : uint8_t { Reading, New, Finished };

  struct ShelfBook {
    RecentBook book;
    ShelfState state = ShelfState::New;
    int percentage = 0;
    uint64_t addedAt = 0;
    std::string banner = "NEW";
    std::string coverThumbPath;
  };

  static constexpr uint8_t GRID_COLUMNS = 3;
  static constexpr int16_t COVER_WIDTH = 110;
  static constexpr int16_t COVER_HEIGHT = 166;
  static constexpr int16_t GRID_GAP = 8;
  static constexpr int16_t GRID_ROW_HEIGHT = 214;

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
  uint16_t gridVisibleCells = GRID_COLUMNS;

  void loadBooks();
  void repairFinishedPaths();
  void loadReadingState(ShelfBook& shelfBook);
  void ensureCoverThumb(ShelfBook& shelfBook);
  void selectIndex(int index);
  void ensureSelectionVisible();
  void promptRemoveBook(const std::string& path, const std::string& title);

  static bool paintCover(freeink::ui::DrawTarget& target, freeink::ui::Rect rect,
                         const freeink::ui::CoverGridItem& item, uint16_t index, void* userData);
};
