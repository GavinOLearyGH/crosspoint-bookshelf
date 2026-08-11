#include "BookActionsActivity.h"

#include <I18n.h>

#include "components/UITheme.h"
#include "components/UiAppHelpers.h"

namespace fui = freeink::ui;

BookActionsActivity::BookActionsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, const bool onBookshelf)
    : UiListActivity("BookActions", renderer, mappedInput), onBookshelf(onBookshelf) {}

void BookActionsActivity::activateIndex(const int index) {
  app.clearTapFlash();
  ActivityResult res{MenuResult{index}};
  res.isCancelled = false;
  setResult(std::move(res));
  finish();
}

bool BookActionsActivity::handleButtons() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    activateIndex(nav.selected);
    return true;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    ActivityResult res;
    res.isCancelled = true;
    setResult(std::move(res));
    finish();
    return true;
  }
  return false;
}

void BookActionsActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  const char* shelfLabel = onBookshelf ? "Remove from Bookshelf" : "Add to Bookshelf";
  fui::ListItem items[] = {{.label = "Open", .actionValue = OPEN},
                           {.label = shelfLabel, .actionValue = TOGGLE_BOOKSHELF},
                           {.label = "Delete from Device", .actionValue = DELETE_FROM_DEVICE}};

  fui::ListProps props;
  props.items = items;
  props.count = 3;
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  syncListViewport(screen, props);
  screen.list(props);
}
