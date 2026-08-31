#pragma once

#include "activities/UiListActivity.h"

class TipPracticeActivity : public UiListActivity {
 public:
  TipPracticeActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;

 protected:
  int listCount() const override { return 4; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return "PRACTICE"; }
};
