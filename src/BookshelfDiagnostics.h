#pragma once

#ifndef BOOKSHELF_MEMORY_DIAGNOSTICS
#define BOOKSHELF_MEMORY_DIAGNOSTICS 0
#endif

#if BOOKSHELF_MEMORY_DIAGNOSTICS
#include <esp_heap_caps.h>
#include "Logging.h"
inline void logBookshelfMemory(const char* stage, const size_t bookCount = 0) {
  const uint32_t heapFree = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  const uint32_t heapMin = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  const uint32_t heapLargest = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  const uint32_t psramFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  const uint32_t psramMin = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  const uint32_t psramLargest = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  LOG_INF("SHELF_MEM", "%s books=%u heap=%u min=%u largest=%u psram=%u psram_min=%u psram_largest=%u", stage,
          static_cast<unsigned>(bookCount), static_cast<unsigned>(heapFree), static_cast<unsigned>(heapMin),
          static_cast<unsigned>(heapLargest), static_cast<unsigned>(psramFree), static_cast<unsigned>(psramMin),
          static_cast<unsigned>(psramLargest));
}
#else
inline void logBookshelfMemory(const char*, size_t = 0) {}
#endif
