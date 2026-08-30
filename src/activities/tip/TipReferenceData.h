#pragma once

#include <string>
#include <vector>

struct TipReferenceItem {
  const char* label;
  const char* detail;
};

struct TipReferenceSection {
  const char* title;
  std::vector<TipReferenceItem> items;
};

namespace TipReferenceData {
const std::vector<TipReferenceSection>& sections();
}
