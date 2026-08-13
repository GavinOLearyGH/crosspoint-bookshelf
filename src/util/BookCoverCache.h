#pragma once

#include <cstdint>
#include <string>

// Ensures a cached cover thumbnail exists for a supported book without
// creating or modifying reading progress. Returns true when a thumbnail is
// available after the call.
bool prepareBookCover(const std::string& path, int16_t height);
