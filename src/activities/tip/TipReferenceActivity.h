#pragma once

#include <string>

#include "activities/UiListActivity.h"

class TipReferenceActivity final : public UiListActivity {
 public:
  TipReferenceActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, int sectionIndex);

 private:
  int sectionIndex;
  std::string title;

  int listCount() const override;
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return title.c_str(); }
};
