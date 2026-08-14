#include "BookActionsActivity.h"

#include <I18n.h>

#include "components/UITheme.h"
#include "components/UiAppHelpers.h"

namespace fui = freeink::ui;

BookActionsActivity::BookActionsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, const bool onBookshelf,
                                         const bool finished, const bool shelfContext)
    : UiListActivity("BookActions", renderer, mappedInput),
      onBookshelf(onBookshelf),
      finished(finished),
      shelfContext(shelfContext) {}

void BookActionsActivity::activateIndex(const int index) {
  app.clearTapFlash();

  int action = index;
  if (!shelfContext) {
    action = index == 0 ? OPEN : (index == 1 ? TOGGLE_BOOKSHELF : DELETE_FROM_LIBRARY);
  }

  ActivityResult res{MenuResult{action}};
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
  const char* finishedLabel = finished ? "Mark Unread" : "Mark Finished";

  fui::ListItem shelfItems[] = {{.label = "Open", .actionValue = OPEN},
                                {.label = finishedLabel, .actionValue = TOGGLE_FINISHED},
                                {.label = shelfLabel, .actionValue = TOGGLE_BOOKSHELF},
                                {.label = "Delete from Library", .actionValue = DELETE_FROM_LIBRARY}};
  fui::ListItem browserItems[] = {{.label = "Open", .actionValue = 0},
                                  {.label = shelfLabel, .actionValue = 1},
                                  {.label = "Delete from Library", .actionValue = 2}};

  fui::ListProps props;
  props.items = shelfContext ? shelfItems : browserItems;
  props.count = shelfContext ? 4 : 3;
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  syncListViewport(screen, props);
  screen.list(props);
}
