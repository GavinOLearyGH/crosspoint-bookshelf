#include "TipPocketRefActivity.h"

#include <memory>
#include <vector>

#include "TipReferenceActivity.h"
#include "TipReferenceData.h"
#include "activities/ActivityManager.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

TipPocketRefActivity::TipPocketRefActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("TipPocketRef", renderer, mappedInput) {}

int TipPocketRefActivity::listCount() const { return static_cast<int>(TipReferenceData::sections().size()); }

void TipPocketRefActivity::activateIndex(const int index) {
  if (index < 0 || index >= listCount()) return;
  nav.selected = index;
  app.clearTapFlash();
  activityManager.pushActivity(std::make_unique<TipReferenceActivity>(renderer, mappedInput, index));
}

void TipPocketRefActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  const auto& sections = TipReferenceData::sections();
  std::vector<fui::ListItem> items;
  items.reserve(sections.size());
  for (size_t i = 0; i < sections.size(); ++i) {
    fui::ListItem item;
    item.label = sections[i].title;
    item.subtitle = "Open reference card";
    item.actionValue = static_cast<int16_t>(i);
    items.push_back(item);
  }

  fui::ListProps props;
  props.items = items.data();
  props.count = static_cast<uint16_t>(items.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  syncListViewport(screen, props);
  screen.list(props);
}
