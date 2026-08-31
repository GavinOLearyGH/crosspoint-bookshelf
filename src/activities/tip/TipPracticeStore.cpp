#include "TipPracticeStore.h"

#include <algorithm>

void TipPracticeStore::toJson(JsonDocument& doc) const {
  doc["active"] = active;
  doc["drill"] = static_cast<uint8_t>(drill);
  doc["resumeOnWake"] = resumeOnWake;
  doc["currentYardage"] = currentYardage;
  doc["attempts"] = attempts;
  doc["hits"] = hits;
  doc["misses"] = misses;
  doc["sequence"] = sequence;
  doc["structuredActive"] = structuredActive;
  doc["plan"] = static_cast<uint8_t>(plan);
  doc["blockIndex"] = blockIndex;
  doc["completedSessions"] = completedSessions;
  doc["lastAttempts"] = lastAttempts;
  doc["lastHits"] = lastHits;
  doc["lastMisses"] = lastMisses;
}

bool TipPracticeStore::fromJson(JsonVariantConst doc) {
  active = doc["active"] | false;
  const uint8_t storedDrill = doc["drill"] | static_cast<uint8_t>(Drill::None);
  drill = storedDrill == static_cast<uint8_t>(Drill::RandomYardage) ? Drill::RandomYardage : Drill::None;
  resumeOnWake = doc["resumeOnWake"] | false;
  currentYardage = std::clamp(doc["currentYardage"] | 100, 70, 180);
  attempts = doc["attempts"] | static_cast<uint16_t>(0);
  hits = doc["hits"] | static_cast<uint16_t>(0);
  misses = doc["misses"] | static_cast<uint16_t>(0);
  sequence = doc["sequence"] | static_cast<uint32_t>(1);
  if (sequence == 0) sequence = 1;

  structuredActive = doc["structuredActive"] | false;
  const uint8_t storedPlan = doc["plan"] | static_cast<uint8_t>(Plan::None);
  switch (storedPlan) {
    case static_cast<uint8_t>(Plan::Min15):
      plan = Plan::Min15;
      break;
    case static_cast<uint8_t>(Plan::Min30):
      plan = Plan::Min30;
      break;
    case static_cast<uint8_t>(Plan::Min60):
      plan = Plan::Min60;
      break;
    default:
      plan = Plan::None;
      break;
  }
  blockIndex = doc["blockIndex"] | static_cast<uint8_t>(0);

  completedSessions = doc["completedSessions"] | static_cast<uint16_t>(0);
  lastAttempts = doc["lastAttempts"] | static_cast<uint16_t>(0);
  lastHits = doc["lastHits"] | static_cast<uint16_t>(0);
  lastMisses = doc["lastMisses"] | static_cast<uint16_t>(0);

  if (hits + misses > attempts) attempts = hits + misses;
  if (active && drill == Drill::None) active = false;
  if (!active || drill != Drill::RandomYardage) resumeOnWake = false;

  const uint8_t count = blockCountForPlan(plan);
  if (!structuredActive || plan == Plan::None || count == 0) {
    structuredActive = false;
    plan = Plan::None;
    blockIndex = 0;
  } else if (blockIndex >= count) {
    blockIndex = count - 1;
  }
  return true;
}

void TipPracticeStore::startRandomYardage() {
  active = true;
  drill = Drill::RandomYardage;
  resumeOnWake = false;
  attempts = 0;
  hits = 0;
  misses = 0;
  currentYardage = nextRandomYardage();
  saveToFile();
}

void TipPracticeStore::recordRandomYardage(const bool hit) {
  if (!active || drill != Drill::RandomYardage) return;
  attempts++;
  if (hit) {
    hits++;
  } else {
    misses++;
  }
  currentYardage = nextRandomYardage();
  saveToFile();
}

void TipPracticeStore::skipRandomYardage() {
  if (!active || drill != Drill::RandomYardage) return;
  currentYardage = nextRandomYardage();
  saveToFile();
}

void TipPracticeStore::finishSession() {
  if (!active) return;
  lastAttempts = attempts;
  lastHits = hits;
  lastMisses = misses;
  completedSessions++;
  active = false;
  drill = Drill::None;
  resumeOnWake = false;
  saveToFile();
}

void TipPracticeStore::startStructuredPlan(const Plan selectedPlan) {
  if (selectedPlan == Plan::None) return;
  structuredActive = true;
  plan = selectedPlan;
  blockIndex = 0;
  saveToFile();
}

void TipPracticeStore::nextStructuredBlock() {
  if (!structuredActive) return;
  const uint8_t count = structuredBlockCount();
  if (count == 0) return;
  if (blockIndex + 1 < count) {
    blockIndex++;
    saveToFile();
  }
}

void TipPracticeStore::previousStructuredBlock() {
  if (!structuredActive || blockIndex == 0) return;
  blockIndex--;
  saveToFile();
}

void TipPracticeStore::finishStructuredPlan() {
  if (!structuredActive) return;
  structuredActive = false;
  plan = Plan::None;
  blockIndex = 0;
  saveToFile();
}

void TipPracticeStore::setResumeOnWake(const bool shouldResume) {
  const bool nextValue = shouldResume && active && drill == Drill::RandomYardage;
  if (resumeOnWake == nextValue) return;
  resumeOnWake = nextValue;
  saveToFile();
}

bool TipPracticeStore::consumeResumeOnWake() {
  if (!resumeOnWake || !active || drill != Drill::RandomYardage) return false;
  resumeOnWake = false;
  saveToFile();
  return true;
}

uint8_t TipPracticeStore::structuredBlockCount() const { return blockCountForPlan(plan); }

uint8_t TipPracticeStore::blockCountForPlan(const Plan selectedPlan) {
  switch (selectedPlan) {
    case Plan::Min15:
      return 3;
    case Plan::Min30:
      return 5;
    case Plan::Min60:
      return 6;
    default:
      return 0;
  }
}

int TipPracticeStore::nextRandomYardage() {
  const int previous = currentYardage;
  for (int tries = 0; tries < 3; ++tries) {
    sequence = sequence * 1664525u + 1013904223u;
    const int yardage = 70 + static_cast<int>((sequence >> 16) % 12u) * 10;
    if (yardage != previous) return yardage;
  }
  return previous == 180 ? 170 : previous + 10;
}
