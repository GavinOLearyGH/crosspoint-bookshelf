#include "BookCoverCache.h"

#include <Epub.h>
#include <FsHelpers.h>
#include <HalStorage.h>
#include <Xtc.h>

bool prepareBookCover(const std::string& path, const int16_t height) {
  if (FsHelpers::hasEpubExtension(path)) {
    Epub epub(path, "/.crosspoint");
    epub.load(false, true);
    const std::string thumbPath = epub.getThumbBmpPath();
    if (!thumbPath.empty() && Storage.exists(thumbPath.c_str())) return true;
    epub.generateThumbBmp(height);
    const std::string generatedPath = epub.getThumbBmpPath();
    return !generatedPath.empty() && Storage.exists(generatedPath.c_str());
  }

  if (FsHelpers::hasXtcExtension(path)) {
    Xtc xtc(path, "/.crosspoint");
    if (!xtc.load()) return false;
    const std::string thumbPath = xtc.getThumbBmpPath();
    if (!thumbPath.empty() && Storage.exists(thumbPath.c_str())) return true;
    xtc.generateThumbBmp(height);
    const std::string generatedPath = xtc.getThumbBmpPath();
    return !generatedPath.empty() && Storage.exists(generatedPath.c_str());
  }

  return false;
}
