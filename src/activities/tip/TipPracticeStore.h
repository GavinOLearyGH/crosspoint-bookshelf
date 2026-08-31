#pragma once

#include <ArduinoJson.h>
#include <PersistableStore.h>

#include <cstdint>

class TipPracticeStore : public PersistableStore<TipPracticeStore> {
  TipPracticeStore() = default;

  friend class PersistableStore<TipPracticeStore>;

 public:
  enum class Drill : uint8_t { None = 0, RandomYardage = 1 };

  static const char* getFilePath() { return "/.crosspoint/tip-practice.json"; }

  void toJson(JsonDocument& doc) const;
  bool fromJson(JsonVariantConst doc);

  void startRandomYardage();
  void recordRandomYardage(bool hit);
  void skipRandomYardage();
  void finishSession();
  void setResumeOnWake(bool shouldResume);
  bool consumeResumeOnWake();

  bool hasActiveSession() const { return active; }
  Drill activeDrill() const { return drill; }
  bool shouldResumeOnWake() const { return resumeOnWake; }
  int targetYardage() const { return currentYardage; }
  uint16_t attemptsCount() const { return attempts; }
  uint16_t hitsCount() const { return hits; }
  uint16_t missesCount() const { return misses; }
  uint16_t sessionsCompleted() const { return completedSessions; }
  uint16_t lastAttemptsCount() const { return lastAttempts; }
  uint16_t lastHitsCount() const { return lastHits; }
  uint16_t lastMissesCount() const { return lastMisses; }

 private:
  int nextRandomYardage();

  bool active = false;
  Drill drill = Drill::None;
  bool resumeOnWake = false;
  int currentYardage = 100;
  uint16_t attempts = 0;
  uint16_t hits = 0;
  uint16_t misses = 0;
  uint32_t sequence = 1;

  uint16_t completedSessions = 0;
  uint16_t lastAttempts = 0;
  uint16_t lastHits = 0;
  uint16_t lastMisses = 0;
};

#define TIP_PRACTICE TipPracticeStore::getInstance()
