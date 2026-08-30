#include "TipHomeActivity.h"

#include <GfxRenderer.h>

#include <memory>

#include "TipPocketRefActivity.h"
#include "activities/ActivityManager.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

TipHomeActivity::TipHomeActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("TipHome", renderer, mappedInput) {}

void TipHomeActivity::onBackButton() { activityManager.goHome(HomeMenuItem::TIP); }

void TipHomeActivity::activateIndex(const int index) {
  nav.selected = index;
  app.clearTapFlash();

  switch (index) {
    case 3:
      activityManager.pushActivity(std::make_unique<TipPocketRefActivity>(renderer, mappedInput));
      break;
    case 5:
      activityManager.goHome(HomeMenuItem::TIP);
      break;
    default:
      // Today, Play, Practice and Golfer are visible architecture anchors in Phase 1.
      // Their interactive behavior is intentionally added in later phases.
      break;
  }
}

void TipHomeActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));
  screen.centeredText("GOLF COMPANION", screen.theme().smallText);
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  static const fui::ListItem items[] = {
      {.label = "Today", .subtitle = "Your current focus and cue - later phase"},
      {.label = "Play", .subtitle = "Round companion - later phase"},
      {.label = "Practice", .subtitle = "Structured sessions - next milestone"},
      {.label = "Pocket Ref", .subtitle = "AS I GO, Sub-5, drills, warm-up and more"},
      {.label = "Golfer", .subtitle = "Bag, yardages and personal state - later phase"},
      {.label = "Read", .subtitle = "Return to Bookshelf, Library and CrossPoint"},
  };

  fui::ListProps props;
  props.items = items;
  props.count = 6;
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  syncListViewport(screen, props);
  screen.list(props);
}
