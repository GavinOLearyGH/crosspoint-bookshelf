#pragma once

#ifndef BOOKSHELF_MEMORY_DIAGNOSTICS
#define BOOKSHELF_MEMORY_DIAGNOSTICS 1
#endif

#if BOOKSHELF_MEMORY_DIAGNOSTICS
#include <Arduino.h>
#include <HalStorage.h>

#include <cstdio>

#include "Logging.h"

namespace BookshelfDiagnostics {
constexpr const char* LOG_PATH = "/bookshelf-memory.log";
constexpr size_t MAX_LOG_BYTES = 512 * 1024;

inline void appendMemoryLog(const char* stage, const size_t bookCount, const uint32_t heap, const uint32_t minHeap,
                            const uint32_t maxAlloc, const uint32_t psram) {
  static uint32_t sequence = 0;

  auto file = Storage.open(LOG_PATH, O_WRONLY | O_CREAT | O_APPEND);
  if (!file) return;

  if (file.size() > MAX_LOG_BYTES) {
    file.close();
    Storage.remove(LOG_PATH);
    file = Storage.open(LOG_PATH, O_WRONLY | O_CREAT | O_APPEND);
    if (!file) return;
    sequence = 0;
  }

  char line[224];
  if (sequence == 0) {
    const int headerLength = snprintf(line, sizeof(line), "--- SHELF_MEM session start millis=%lu ---\n",
                                      static_cast<unsigned long>(millis()));
    if (headerLength > 0) file.write(line, static_cast<size_t>(headerLength));
  }

  ++sequence;
  const int lineLength = snprintf(
      line, sizeof(line), "%06lu ms=%lu stage=%s books=%u heap=%u min=%u max_alloc=%u psram=%u\n",
      static_cast<unsigned long>(sequence), static_cast<unsigned long>(millis()), stage,
      static_cast<unsigned>(bookCount), static_cast<unsigned>(heap), static_cast<unsigned>(minHeap),
      static_cast<unsigned>(maxAlloc), static_cast<unsigned>(psram));
  if (lineLength > 0) file.write(line, static_cast<size_t>(lineLength));
  file.flush();
  file.close();
}
}  // namespace BookshelfDiagnostics

inline void logBookshelfMemory(const char* stage, const size_t bookCount = 0) {
  const uint32_t heap = ESP.getFreeHeap();
  const uint32_t minHeap = ESP.getMinFreeHeap();
  const uint32_t maxAlloc = ESP.getMaxAllocHeap();
  const uint32_t psram = ESP.getFreePsram();

  LOG_INF("SHELF_MEM", "%s books=%u heap=%u min=%u max_alloc=%u psram=%u", stage, static_cast<unsigned>(bookCount),
          static_cast<unsigned>(heap), static_cast<unsigned>(minHeap), static_cast<unsigned>(maxAlloc),
          static_cast<unsigned>(psram));
  BookshelfDiagnostics::appendMemoryLog(stage, bookCount, heap, minHeap, maxAlloc, psram);
}
#else
inline void logBookshelfMemory(const char*, size_t = 0) {}
#endif
