#include "BootActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <array>

#include "fontIds.h"
#include "images/BookshelfLogo120.h"

namespace {
constexpr int BOOKSHELF_LOGO_SIZE = 120;
constexpr int BOOKSHELF_LOGO_ROW_BYTES = BOOKSHELF_LOGO_SIZE / 8;
constexpr int BOOKSHELF_LOGO_BYTES = BOOKSHELF_LOGO_SIZE * BOOKSHELF_LOGO_ROW_BYTES;

std::array<uint8_t, BOOKSHELF_LOGO_BYTES> rotateBookshelfLogoCounterClockwise() {
  std::array<uint8_t, BOOKSHELF_LOGO_BYTES> rotated{};
  rotated.fill(0xFF);

  for (int y = 0; y < BOOKSHELF_LOGO_SIZE; ++y) {
    for (int x = 0; x < BOOKSHELF_LOGO_SIZE; ++x) {
      const int sourceX = BOOKSHELF_LOGO_SIZE - 1 - y;
      const int sourceY = x;
      const int sourceIndex = sourceY * BOOKSHELF_LOGO_ROW_BYTES + sourceX / 8;
      const uint8_t sourceMask = static_cast<uint8_t>(0x80U >> (sourceX % 8));

      if ((BookshelfLogo120[sourceIndex] & sourceMask) == 0) {
        const int destinationIndex = y * BOOKSHELF_LOGO_ROW_BYTES + x / 8;
        const uint8_t destinationMask = static_cast<uint8_t>(0x80U >> (x % 8));
        rotated[destinationIndex] &= static_cast<uint8_t>(~destinationMask);
      }
    }
  }

  return rotated;
}
}  // namespace

void BootActivity::onEnter() {
  Activity::onEnter();

  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  const auto bookshelfLogo = rotateBookshelfLogoCounterClockwise();

  renderer.clearScreen();
  renderer.drawImage(bookshelfLogo.data(), (pageWidth - 120) / 2, (pageHeight - 120) / 2, 120, 120);
  renderer.drawCenteredText(UI_10_FONT_ID, pageHeight / 2 + 70, tr(STR_CROSSPOINT), true, EpdFontFamily::BOLD);
  renderer.drawCenteredText(SMALL_FONT_ID, pageHeight / 2 + 95, "BOOKSHELF EDITION");
  renderer.drawCenteredText(SMALL_FONT_ID, pageHeight - 30, CROSSPOINT_VERSION);
  renderer.displayBuffer();
}