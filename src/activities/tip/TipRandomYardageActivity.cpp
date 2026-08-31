#include "TipRandomYardageActivity.h"

#include "TipPracticeStore.h"
#include "activities/ActivityManager.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

TipRandomYardageActivity::TipRandomYardageActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("TipRandomYardage", renderer, mappedInput) {}

void TipRandomYardageActivity::onEnter() {
  TIP_PRACTICE.loadFromFile();
  if (!TIP_PRACTICE.hasActiveSession() || TIP_PRACTICE.activeDrill() != TipPracticeStore::Drill::RandomYardage) {
    TIP_PRACTICE.startRandomYardage();
  }
  UiListActivity::onEnter();
}

void TipRandomYardageActivity::activateIndex(const int index) {
  nav.selected = index;
  app.clearTapFlash();

  switch (index) {
    case 0:
      TIP_PRACTICE.recordRandomYardage(true);
      requestUpdate();
      break;
    case 1:
      TIP_PRACTICE.recordRandomYardage(false);
      requestUpdate();
      break;
    case 2:
      TIP_PRACTICE.skipRandomYardage();
      requestUpdate();
      break;
    case 3:
      TIP_PRACTICE.finishSession();
      finish();
      break;
    default:
      break;
  }
}

void TipRandomYardageActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  targetLine = "TARGET  " + std::to_string(TIP_PRACTICE.targetYardage()) + " YDS";
  statsLine = "Shots " + std::to_string(TIP_PRACTICE.attemptsCount()) + "   Hit " +
              std::to_string(TIP_PRACTICE.hitsCount()) + "   Miss " + std::to_string(TIP_PRACTICE.missesCount());

  screen.centeredText(targetLine.c_str(), screen.theme().bodyText);
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));
  screen.centeredText(statsLine.c_str(), screen.theme().smallText);
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  static const fui::ListItem items[] = {
      {.label = "Hit", .subtitle = "Record target achieved"},
      {.label = "Miss", .subtitle = "Record target missed"},
      {.label = "New Target", .subtitle = "Skip without recording a shot"},
      {.label = "Finish", .subtitle = "Save summary and end session"},
  };

  fui::ListProps props;
  props.items = items;
  props.count = 4;
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  syncListViewport(screen, props);
  screen.list(props);
}
