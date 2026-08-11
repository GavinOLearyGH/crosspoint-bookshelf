#pragma once

#include <ArduinoJson.h>
#include <PersistableStore.h>

#include <string>
#include <vector>

struct BookshelfEntry {
  std::string path;
  uint64_t addedAt = 0;
};

class BookshelfStore : public PersistableStore<BookshelfStore> {
 private:
  std::vector<BookshelfEntry> entries;
  bool loaded = false;

  BookshelfStore() = default;
  ~BookshelfStore() = default;

  friend class PersistableStore<BookshelfStore>;

  void ensureLoaded();

 public:
  static const char* getFilePath() { return "/.crosspoint/bookshelf.json"; }

  void toJson(JsonDocument& doc) const;
  bool fromJson(JsonVariantConst doc);

  bool contains(const std::string& path);
  bool add(const std::string& path, uint64_t addedAt = 0);
  bool remove(const std::string& path);
  bool updatePath(const std::string& oldPath, const std::string& newPath);
  bool pruneMissing();

  const std::vector<BookshelfEntry>& getEntries() {
    ensureLoaded();
    return entries;
  }
  int getCount() const { return static_cast<int>(entries.size()); }
};

#define BOOKSHELF BookshelfStore::getInstance()
