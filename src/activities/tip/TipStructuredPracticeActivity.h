#pragma once

#include <string>

#include "TipPracticeStore.h"
#include "activities/UiListActivity.h"

class TipStructuredPracticeActivity : public UiListActivity {
 public:
  TipStructuredPracticeActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, TipPracticeStore::Plan plan);
  void onEnter() override;

 protected:
  int listCount() const override { return 4; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return "PRACTICE PLAN"; }

 private:
  struct Block {
    const char* label;
    const char* cue;
    uint8_t minutes;
    bool randomYardage;
  };

  const Block* blocks() const;
  uint8_t blockCount() const;
  const Block& currentBlock() const;

  TipPracticeStore::Plan requestedPlan;
  std::string progressLine;
  std::string blockLine;
  std::string cueLine;
};
