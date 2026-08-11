#include "BookshelfActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "components/UITheme.h"
#include "fontIds.h"

void BookshelfActivity::onEnter() {
  Activity::onEnter();
  requestUpdate();
}

void BookshelfActivity::loop() {
  int x = 0;
  int y = 0;
  if (mappedInput.wasReleased(MappedInputManager::Button::Back) || mappedInput.wasScreenTapped(x, y)) {
    finish();
  }
}

void BookshelfActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, "Bookshelf");

  const int y = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing * 3;
  renderer.drawCenteredText(UI_10_FONT_ID, y, "Your Bookshelf");

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
