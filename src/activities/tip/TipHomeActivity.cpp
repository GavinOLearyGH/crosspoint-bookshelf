#include "TipHomeActivity.h"

#include <GfxRenderer.h>

#include <memory>
#include <vector>

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
      startActivity(std::make_unique<TipPocketRefActivity>(renderer, mappedInput));
      break;
    case 5:
      activityManager.goHome(HomeMenuItem::TIP);
      break;
    case 0:
      activityManager.goToFullScreenMessage("TODAY\n\nGolfer sync arrives in a later phase.");
      break;
    case 1:
      activityManager.goToFullScreenMessage("PLAY\n\nRound companion arrives after the shell is proven.");
      break;
    case 2:
      activityManager.goToFullScreenMessage("PRACTICE\n\nInteractive practice is the next build milestone.");
      break;
    case 4:
      activityManager.goToFullScreenMessage("GOLFER\n\nPersonal golfer data arrives with local golfer state.");
      break;
    default:
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
      {.label = "Today", .subtitle = "Your current focus and cue"},
      {.label = "Play", .subtitle = "Round companion"},
      {.label = "Practice", .subtitle = "Drills and structured sessions"},
      {.label = "Pocket Ref", .subtitle = "AS I GO, Sub-5, drills, warm-up and more"},
      {.label = "Golfer", .subtitle = "Bag, yardages and personal state"},
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
