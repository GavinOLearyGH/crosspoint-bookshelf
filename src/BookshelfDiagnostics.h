#pragma once

#ifndef BOOKSHELF_MEMORY_DIAGNOSTICS
#define BOOKSHELF_MEMORY_DIAGNOSTICS 1
#endif

#if BOOKSHELF_MEMORY_DIAGNOSTICS
#include <Arduino.h>

#include "Logging.h"

inline void logBookshelfMemory(const char* stage, const size_t bookCount = 0) {
  LOG_INF("SHELF_MEM", "%s books=%u heap=%u min=%u max_alloc=%u psram=%u", stage,
          static_cast<unsigned>(bookCount), static_cast<unsigned>(ESP.getFreeHeap()),
          static_cast<unsigned>(ESP.getMinFreeHeap()), static_cast<unsigned>(ESP.getMaxAllocHeap()),
          static_cast<unsigned>(ESP.getFreePsram()));
}
#else
inline void logBookshelfMemory(const char*, size_t = 0) {}
#endif
