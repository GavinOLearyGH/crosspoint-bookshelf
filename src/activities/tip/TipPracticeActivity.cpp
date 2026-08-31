#include "TipPracticeActivity.h"

#include <memory>

#include "TipPracticeStore.h"
#include "TipRandomYardageActivity.h"
#include "TipStructuredPracticeActivity.h"
#include "activities/ActivityManager.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

TipPracticeActivity::TipPracticeActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("TipPractice", renderer, mappedInput) {}

void TipPracticeActivity::onEnter() {
  TIP_PRACTICE.loadFromFile();
  UiListActivity::onEnter();
}

void TipPracticeActivity::activateIndex(const int index) {
  nav.selected = index;
  app.clearTapFlash();

  switch (index) {
    case 0:
      activityManager.pushActivity(std::make_unique<TipRandomYardageActivity>(renderer, mappedInput));
      break;
    case 1:
      activityManager.pushActivity(
          std::make_unique<TipStructuredPracticeActivity>(renderer, mappedInput, TipPracticeStore::Plan::Min15));
      break;
    case 2:
      activityManager.pushActivity(
          std::make_unique<TipStructuredPracticeActivity>(renderer, mappedInput, TipPracticeStore::Plan::Min30));
      break;
    case 3:
      activityManager.pushActivity(
          std::make_unique<TipStructuredPracticeActivity>(renderer, mappedInput, TipPracticeStore::Plan::Min60));
      break;
    default:
      break;
  }
}

void TipPracticeActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  const char* randomSubtitle =
      TIP_PRACTICE.hasActiveSession() ? "Resume current session" : "Start first live TIP drill";
  const auto activePlan = TIP_PRACTICE.hasActiveStructuredPlan() ? TIP_PRACTICE.structuredPlan() : TipPracticeStore::Plan::None;
  const fui::ListItem items[] = {
      {.label = "Random Yardage", .subtitle = randomSubtitle},
      {.label = "15 Minute", .subtitle = activePlan == TipPracticeStore::Plan::Min15 ? "Resume structured session" : "Quick focused practice"},
      {.label = "30 Minute", .subtitle = activePlan == TipPracticeStore::Plan::Min30 ? "Resume structured session" : "Balanced practice session"},
      {.label = "60 Minute", .subtitle = activePlan == TipPracticeStore::Plan::Min60 ? "Resume structured session" : "Full practice session"},
  };

  fui::ListProps props;
  props.items = items;
  props.count = 4;
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  syncListViewport(screen, props);
  screen.list(props);
}
