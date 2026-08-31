#include "TipPracticeStore.h"

#include <algorithm>

void TipPracticeStore::toJson(JsonDocument& doc) const {
  doc["active"] = active;
  doc["drill"] = static_cast<uint8_t>(drill);
  doc["currentYardage"] = currentYardage;
  doc["attempts"] = attempts;
  doc["hits"] = hits;
  doc["misses"] = misses;
  doc["sequence"] = sequence;
  doc["completedSessions"] = completedSessions;
  doc["lastAttempts"] = lastAttempts;
  doc["lastHits"] = lastHits;
  doc["lastMisses"] = lastMisses;
}

bool TipPracticeStore::fromJson(JsonVariantConst doc) {
  active = doc["active"] | false;
  const uint8_t storedDrill = doc["drill"] | static_cast<uint8_t>(Drill::None);
  drill = storedDrill == static_cast<uint8_t>(Drill::RandomYardage) ? Drill::RandomYardage : Drill::None;
  currentYardage = std::clamp(doc["currentYardage"] | 100, 70, 180);
  attempts = doc["attempts"] | static_cast<uint16_t>(0);
  hits = doc["hits"] | static_cast<uint16_t>(0);
  misses = doc["misses"] | static_cast<uint16_t>(0);
  sequence = doc["sequence"] | static_cast<uint32_t>(1);
  if (sequence == 0) sequence = 1;
  completedSessions = doc["completedSessions"] | static_cast<uint16_t>(0);
  lastAttempts = doc["lastAttempts"] | static_cast<uint16_t>(0);
  lastHits = doc["lastHits"] | static_cast<uint16_t>(0);
  lastMisses = doc["lastMisses"] | static_cast<uint16_t>(0);

  if (hits + misses > attempts) attempts = hits + misses;
  if (active && drill == Drill::None) active = false;
  return true;
}

void TipPracticeStore::startRandomYardage() {
  active = true;
  drill = Drill::RandomYardage;
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
  saveToFile();
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
