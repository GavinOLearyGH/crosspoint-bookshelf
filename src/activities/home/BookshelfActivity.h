#pragma once

#include "activities/Activity.h"

class BookshelfActivity final : public Activity {
 public:
  BookshelfActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Bookshelf", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
