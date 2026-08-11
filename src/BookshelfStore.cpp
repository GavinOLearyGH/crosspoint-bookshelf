#include "BookshelfStore.h"

#include <HalStorage.h>
#include <Logging.h>

#include <algorithm>

void BookshelfStore::ensureLoaded() {
  if (loaded) return;
  loaded = true;
  loadFromFile();
}

void BookshelfStore::toJson(JsonDocument& doc) const {
  JsonArray arr = doc["books"].to<JsonArray>();
  for (const auto& entry : entries) {
    JsonObject obj = arr.add<JsonObject>();
    obj["path"] = entry.path;
    obj["addedAt"] = entry.addedAt;
  }
}

bool BookshelfStore::fromJson(JsonVariantConst doc) {
  loaded = true;
  entries.clear();
  JsonArrayConst arr = doc["books"].as<JsonArrayConst>();
  entries.reserve(arr.size());

  for (JsonObjectConst obj : arr) {
    const char* path = obj["path"] | "";
    if (path[0] == '\0') continue;
    entries.push_back({path, obj["addedAt"] | 0ULL});
  }

  LOG_DBG("SHELF", "Bookshelf loaded from file (%d entries)", getCount());
  return true;
}

bool BookshelfStore::contains(const std::string& path) {
  ensureLoaded();
  return std::any_of(entries.begin(), entries.end(), [&](const BookshelfEntry& entry) { return entry.path == path; });
}

bool BookshelfStore::add(const std::string& path, const uint64_t addedAt) {
  ensureLoaded();
  if (path.empty() || contains(path)) return false;
  entries.push_back({path, addedAt});
  if (!saveToFile()) LOG_ERR("SHELF", "Failed to persist bookshelf add: %s", path.c_str());
  return true;
}

bool BookshelfStore::remove(const std::string& path) {
  ensureLoaded();
  const auto it =
      std::find_if(entries.begin(), entries.end(), [&](const BookshelfEntry& entry) { return entry.path == path; });
  if (it == entries.end()) return false;
  entries.erase(it);
  if (!saveToFile()) LOG_ERR("SHELF", "Failed to persist bookshelf removal: %s", path.c_str());
  return true;
}

bool BookshelfStore::updatePath(const std::string& oldPath, const std::string& newPath) {
  ensureLoaded();
  auto it =
      std::find_if(entries.begin(), entries.end(), [&](const BookshelfEntry& entry) { return entry.path == oldPath; });
  if (it == entries.end()) return false;
  it->path = newPath;
  if (!saveToFile()) LOG_ERR("SHELF", "Failed to persist bookshelf path update");
  return true;
}

bool BookshelfStore::pruneMissing() {
  ensureLoaded();
  const size_t before = entries.size();
  entries.erase(std::remove_if(entries.begin(), entries.end(),
                               [](const BookshelfEntry& entry) { return !Storage.exists(entry.path.c_str()); }),
                entries.end());
  if (entries.size() == before) return false;
  if (!saveToFile()) LOG_ERR("SHELF", "Failed to persist bookshelf pruning");
  return true;
}
