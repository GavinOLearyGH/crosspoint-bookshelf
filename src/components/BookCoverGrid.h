#pragma once

#include <FreeInkUI.h>

namespace BookCoverGrid {

struct Layout {
  uint8_t columns = 3;
  int16_t coverWidth = 110;
  int16_t coverHeight = 166;
  int16_t rowHeight = 214;
  int16_t gap = 8;
};

inline freeink::ui::CoverGridProps makeProps(const freeink::ui::UiScreen& screen, const Layout& layout,
                                             const uint16_t count, const uint16_t topIndex,
                                             const int16_t selectedIndex,
                                             freeink::ui::CoverGridItemProvider itemProvider,
                                             void* itemProviderUserData,
                                             freeink::ui::CoverGridCoverPainter coverPainter,
                                             void* coverPainterUserData, const freeink::ui::ActionId action,
                                             const uint16_t inputMask) {
  freeink::ui::CoverGridProps props;
  props.itemProvider = itemProvider;
  props.itemProviderUserData = itemProviderUserData;
  props.count = count;
  props.topIndex = topIndex;
  props.selectedIndex = selectedIndex;
  props.action = action;
  props.inputMask = inputMask;
  props.columns = layout.columns;
  props.coverSize = freeink::ui::Size{layout.coverWidth, layout.coverHeight};
  props.rowHeight = layout.rowHeight;
  props.gap = layout.gap;
  props.rowGap = layout.gap;
  props.selectionIndicator = freeink::ui::CoverGridSelectionIndicator::CoverFrame;
  props.titleText = screen.theme().smallText;
  props.titleText.maxLines = 2;
  props.labelHeight = static_cast<int16_t>(screen.target().lineHeight(props.titleText.font) * 2);
  props.labelGap = 4;
  props.coverPainter = coverPainter;
  props.coverPainterUserData = coverPainterUserData;
  return props;
}

inline uint16_t visibleCells(const freeink::ui::Rect rect, const Layout& layout) {
  return freeink::ui::coverGridVisibleCells(rect, layout.columns, layout.rowHeight, layout.gap);
}

}  // namespace BookCoverGrid
