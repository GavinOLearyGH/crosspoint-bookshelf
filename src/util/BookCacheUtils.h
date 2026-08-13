#pragma once

#include <cstdint>
#include <string>

// Clears the reading cache for a book file if its extension is recognised
// (EPUB, XTC, or TXT). Does nothing for other file types.
void clearBookCache(const std::string& path);

// Ensures a cached cover thumbnail exists for a supported book without
// creating or modifying reading progress. Returns true when a thumbnail is
// available after the call.
bool ensureBookCoverThumb(const std::string& path, int16_t height);

// Returns true if the directory name matches a book cache entry.
bool isBookCacheDirectoryName(const char* name);