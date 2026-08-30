#include "TipReferenceActivity.h"

#include <vector>

#include "TipReferenceData.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

TipReferenceActivity::TipReferenceActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, const int sectionIndex)
    : UiListActivity("TipReference", renderer, mappedInput), sectionIndex(sectionIndex) {
  const auto& all = TipReferenceData::sections();
  if (sectionIndex >= 0 && sectionIndex < static_cast<int>(all.size())) {
    title = all[sectionIndex].title;
  } else {
    title = "POCKET REF";
    this->sectionIndex = 0;
  }
}

int TipReferenceActivity::listCount() const {
  const auto& all = TipReferenceData::sections();
  if (sectionIndex < 0 || sectionIndex >= static_cast<int>(all.size())) return 0;
  return static_cast<int>(all[sectionIndex].items.size());
}

void TipReferenceActivity::activateIndex(const int index) {
  // Reference rows are intentionally read-only in Phase 1.
  if (index >= 0 && index < listCount()) {
    nav.selected = index;
    app.clearTapFlash();
    requestUpdate();
  }
}

void TipReferenceActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  const auto& section = TipReferenceData::sections()[sectionIndex];
  std::vector<fui::ListItem> items;
  items.reserve(section.items.size());
  for (size_t i = 0; i < section.items.size(); ++i) {
    fui::ListItem item;
    item.label = section.items[i].label;
    item.subtitle = section.items[i].detail;
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
