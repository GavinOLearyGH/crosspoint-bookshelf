#pragma once

#include "activities/UiListActivity.h"

class TipHomeActivity final : public UiListActivity {
 public:
  TipHomeActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);

 private:
  int listCount() const override { return 6; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  void onBackButton() override;
  const char* headerTitle() const override { return "THE IRISH PAR"; }
};
