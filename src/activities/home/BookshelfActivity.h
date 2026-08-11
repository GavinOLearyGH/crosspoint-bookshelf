#pragma once

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
  int listCount() const override { return static_cast<int>(books.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  void onRowLongPress(int index) override;
  bool handleButtons() override;
  const char* headerTitle() const override { return "Bookshelf"; }
  void drawFooter() override;

  std::vector<RecentBook> books;
  bool longPressFired = false;

  void loadBooks();
  void promptRemoveBook(const std::string& path, const std::string& title);
};
