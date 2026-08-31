#pragma once

#include <string>

#include "activities/UiListActivity.h"

class TipRandomYardageActivity : public UiListActivity {
 public:
  TipRandomYardageActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;

 protected:
  int listCount() const override { return 4; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return "RANDOM YARDAGE"; }

 private:
  std::string targetLine;
  std::string statsLine;
};
