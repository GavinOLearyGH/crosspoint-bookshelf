#pragma once

#include "activities/UiListActivity.h"

class TipPocketRefActivity final : public UiListActivity {
 public:
  TipPocketRefActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);

 private:
  int listCount() const override;
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return "POCKET REF"; }
};
