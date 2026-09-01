#include "TipStructuredPracticeActivity.h"

#include <memory>

#include "TipRandomYardageActivity.h"
#include "activities/ActivityManager.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
constexpr TipStructuredPracticeActivity::Block PLAN_15[] = {
    {"Warm Up", "Mobility then easy half-to-3/4 swings", "GOAL: Find balance and rhythm", 3, false},
    {"Random Yardage", "Random 70-180. AS I GO every ball", "GOAL: Commit before every strike", 8, true},
    {"Pressure Finish", "3 balls. Pick target. Full routine", "GOAL: 2 of 3 playable results", 4, false},
};

constexpr TipStructuredPracticeActivity::Block PLAN_30[] = {
    {"Warm Up", "Mobility then build half-to-full swings", "GOAL: Finish balanced", 5, false},
    {"Contact", "9 balls. Center-face before speed", "GOAL: 6 of 9 solid strikes", 5, false},
    {"Random Yardage", "Random 70-180. AS I GO every ball", "GOAL: Commit and score Hit / Miss", 10, true},
    {"Landing Zone", "Pick spot first. Change club/flight", "GOAL: 3 of 5 in chosen zone", 5, false},
    {"Pressure Finish", "5 balls. New target each shot", "GOAL: 4 of 5 playable results", 5, false},
};

constexpr TipStructuredPracticeActivity::Block PLAN_60[] = {
    {"Warm Up", "Mobility then build half-to-full swings", "GOAL: Rhythm before speed", 8, false},
    {"Contact", "3x3 balls. Center, start line, flight", "GOAL: 2 solid balls in each set", 10, false},
    {"Random Yardage", "Random 70-180. AS I GO every ball", "GOAL: Commit and score Hit / Miss", 15, true},
    {"Landing Zone", "Low, stock, high to chosen zones", "GOAL: Predict landing and rollout", 10, false},
    {"Fairway Finder", "10 tee balls. One side taken out", "GOAL: 7 of 10 playable", 10, false},
    {"Pressure Finish", "7 balls. Full routine. No repeats", "GOAL: 5 of 7 playable results", 7, false},
};
}  // namespace

TipStructuredPracticeActivity::TipStructuredPracticeActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                                             const TipPracticeStore::Plan plan)
    : UiListActivity("TipStructuredPractice", renderer, mappedInput), requestedPlan(plan) {}

void TipStructuredPracticeActivity::onEnter() {
  TIP_PRACTICE.loadFromFile();
  if (!TIP_PRACTICE.hasActiveStructuredPlan() || TIP_PRACTICE.structuredPlan() != requestedPlan) {
    TIP_PRACTICE.startStructuredPlan(requestedPlan);
  }
  UiListActivity::onEnter();
}

void TipStructuredPracticeActivity::activateIndex(const int index) {
  nav.selected = index;
  app.clearTapFlash();

  const auto& block = currentBlock();
  switch (index) {
    case 0:
      if (TIP_PRACTICE.structuredBlockIndex() + 1 >= TIP_PRACTICE.structuredBlockCount()) {
        TIP_PRACTICE.finishStructuredPlan();
        finish();
      } else {
        TIP_PRACTICE.nextStructuredBlock();
        requestUpdate();
      }
      break;
    case 1:
      TIP_PRACTICE.previousStructuredBlock();
      requestUpdate();
      break;
    case 2:
      if (block.randomYardage) {
        activityManager.pushActivity(std::make_unique<TipRandomYardageActivity>(renderer, mappedInput));
      }
      break;
    case 3:
      TIP_PRACTICE.finishStructuredPlan();
      finish();
      break;
    default:
      break;
  }
}

void TipStructuredPracticeActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  const auto& block = currentBlock();
  const auto current = static_cast<unsigned int>(TIP_PRACTICE.structuredBlockIndex() + 1);
  const auto count = static_cast<unsigned int>(TIP_PRACTICE.structuredBlockCount());

  progressLine = "BLOCK " + std::to_string(current) + " / " + std::to_string(count);
  blockLine = std::string(block.label) + "  " + std::to_string(block.minutes) + " MIN";
  cueLine = block.cue;
  goalLine = block.goal;

  screen.centeredText(progressLine.c_str(), screen.theme().smallText);
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));
  screen.centeredText(blockLine.c_str(), screen.theme().bodyText);
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));
  screen.centeredText(cueLine.c_str(), screen.theme().smallText);
  screen.centeredText(goalLine.c_str(), screen.theme().smallText);
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  const bool lastBlock = TIP_PRACTICE.structuredBlockIndex() + 1 >= TIP_PRACTICE.structuredBlockCount();
  const char* nextLabel = lastBlock ? "Complete Plan" : "Next Block";
  const char* drillSubtitle = block.randomYardage ? "Open live Hit / Miss drill" : "Follow prescription above";

  const fui::ListItem items[] = {
      {.label = nextLabel, .subtitle = lastBlock ? "Save progress and finish" : "Mark block complete and continue"},
      {.label = "Previous Block", .subtitle = "Go back one practice block"},
      {.label = block.randomYardage ? "Open Drill" : "Prescription", .subtitle = drillSubtitle},
      {.label = "End Session", .subtitle = "Stop this practice plan"},
  };

  fui::ListProps props;
  props.items = items;
  props.count = 4;
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  syncListViewport(screen, props);
  screen.list(props);
}

const TipStructuredPracticeActivity::Block* TipStructuredPracticeActivity::blocks() const {
  switch (requestedPlan) {
    case TipPracticeStore::Plan::Min15:
      return PLAN_15;
    case TipPracticeStore::Plan::Min30:
      return PLAN_30;
    case TipPracticeStore::Plan::Min60:
      return PLAN_60;
    default:
      return PLAN_15;
  }
}

uint8_t TipStructuredPracticeActivity::blockCount() const {
  switch (requestedPlan) {
    case TipPracticeStore::Plan::Min15:
      return 3;
    case TipPracticeStore::Plan::Min30:
      return 5;
    case TipPracticeStore::Plan::Min60:
      return 6;
    default:
      return 3;
  }
}

const TipStructuredPracticeActivity::Block& TipStructuredPracticeActivity::currentBlock() const {
  uint8_t index = TIP_PRACTICE.structuredBlockIndex();
  const uint8_t count = blockCount();
  if (index >= count) index = count - 1;
  return blocks()[index];
}
