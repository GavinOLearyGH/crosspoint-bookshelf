# CrossPoint Bookshelf

**CrossPoint Bookshelf is an experimental, lightweight Bookshelf extension for [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader) on the XTEINK X4 Pro.**

It adds a curated reading shelf with cover art, reading progress, smart ordering, and a few simple book-management actions while intentionally keeping the underlying CrossPoint reading experience as close to upstream as possible.

> [!IMPORTANT]
> This is an independent community fork and is **not an official CrossPoint Reader release**. CrossPoint Reader and its contributors deserve the credit for the firmware, reader engine, X4 Pro support, UI framework, EPUB handling, and the overwhelming majority of the code in this repository.

## Project status

| | |
|---|---|
| **Bookshelf version** | V1.1 |
| **Target device** | XTEINK X4 Pro |
| **Upstream project** | [crosspoint-reader/crosspoint-reader](https://github.com/crosspoint-reader/crosspoint-reader) |
| **Upstream branch used for the original Bookshelf baseline** | `feat-touch-ui` |
| **Pinned upstream baseline commit** | `b4a240161016ffbc0d8ca4f8917afe870b6c2594` |
| **Baseline date** | August 9, 2026 |
| **Status** | Experimental / tested on a physical X4 Pro |

### A note about the X4 Pro beta version

The Bookshelf work was started from the X4 Pro beta development line and pinned to the exact upstream commit above so the build is reproducible. We initially discussed the public beta by number while it was changing quickly, but the beta number is not encoded in the repository commit itself. Public community discussion on August 11 identified **Beta 19** as the current X4 Pro beta at that time.

For that reason, this README uses the **exact upstream commit SHA** as the authoritative baseline instead of claiming that the fork is based on “Beta 20.” This avoids misrepresenting the upstream project as its beta builds continue to move.

## Why Bookshelf?

CrossPoint already has a good file browser, Recent Books, and quick access to recent reading from Home. What I wanted was one additional screen that answered a simpler question:

> **What are the books I actually want to read?**

The files on the device remain the library. **Bookshelf is a smaller, intentional collection** of the books I am reading now, want to read next, or have finished.

The goal is deliberately not to create a full library-management system. The X4 Pro is a reading device, and Bookshelf should remain lightweight enough that it never gets in the way of reading.

## Home

Bookshelf V1.1 keeps the familiar CrossPoint Home screen and places Bookshelf at the top of the main menu:

```text
[ Recent Book ] [ Recent Book ] [ Recent Book ]

Bookshelf
Browse
Recent
File Transfer
Settings
```

If an OPDS server is configured, CrossPoint's existing OPDS entry is preserved as well.

The three recent covers remain CrossPoint's native Recent Books experience. Bookshelf does not replace Home or the core navigation architecture.

## The Bookshelf

From **Browse**, long-press a book and choose **Add to Bookshelf**.

Adding a book to the Shelf does not move or duplicate the actual book. It simply adds the book's path to a small Bookshelf store on the SD card.

Bookshelf displays selected books in a **three-column cover grid** and uses CrossPoint's existing cached cover artwork and EPUB metadata.

Each cover has a small status banner:

```text
NEW        37%        100%
```

- **NEW** — on the Shelf but not started
- **1–99%** — currently being read
- **100%** — finished

Reading progress is derived from CrossPoint's existing EPUB progress data rather than maintaining a second reading-progress system.

## Smart ordering

Bookshelf automatically keeps the most useful books toward the front:

1. **Currently Reading**
2. **New**
3. **Finished**

Currently-reading books use CrossPoint's existing Recent Books ordering as a lightweight recency signal. There are no Shelf tabs, filters, categories, collections, or sorting screens to manage.

The sorting is intended to be useful without becoming another feature the reader has to think about.

## Book actions

Long-press a cover on the Bookshelf to access:

```text
Open
Mark Finished / Mark Unread
Remove from Shelf
Delete from Library
```

### Remove from Shelf

Removes the book from the curated Bookshelf only. **The actual book remains on the device.**

### Delete from Library

Deletes the book from the device and cleans up its CrossPoint cache, Recent Books entry, and Bookshelf membership.

### Mark Finished

Marks a book as complete and displays `100%` on the Shelf.

### Mark Unread

Clears the completed state and, for EPUBs, resets the saved reading position so the book can genuinely be started again.

## Empty Bookshelf

A new Shelf explains how to populate it rather than presenting an unexplained empty screen:

```text
Your Bookshelf is empty

Long-press a book in Browse
and choose Add to Bookshelf
```

## Design philosophy

The guiding principle for this fork is:

> **CrossPoint remains the firmware; Bookshelf is an add-on.**

Bookshelf therefore tries to reuse CrossPoint rather than replace it:

- CrossPoint's reader engine remains unchanged by V1.1.
- CrossPoint remains responsible for EPUB rendering and reading progress.
- CrossPoint's cover cache and book metadata are reused.
- CrossPoint's Recent Books data is reused for active-reading order.
- Bookshelf has its own small persistent store at `/.crosspoint/bookshelf.json`.
- Bookshelf-specific behavior lives primarily in `BookshelfActivity`, `BookshelfStore`, and `BookActionsActivity`.
- Existing CrossPoint screens are changed only where an integration point is required.

This is intentional. The smaller the integration footprint, the easier it should be to port Bookshelf onto future X4 Pro CrossPoint builds.

## Upstream updates

This fork is **not intended to become a permanently divergent CrossPoint distribution**.

The intended maintenance model is:

```text
New CrossPoint X4 Pro build
          |
          v
Review upstream changes
          |
          v
Port/rebase the small Bookshelf layer
          |
          v
Run full CI + X4 Pro build
          |
          v
Test on physical X4 Pro
```

Bookshelf membership is stored on the SD card rather than compiled into the firmware, so the Shelf data is designed to survive firmware builds and updates.

## Building for X4 Pro

Clone this fork with its submodules:

```bash
git clone --recursive https://github.com/GavinOLearyGH/crosspoint-bookshelf.git
cd crosspoint-bookshelf
```

If the repository was cloned without submodules:

```bash
git submodule update --init --recursive
```

Build the X4 Pro target with PlatformIO:

```bash
pio run -e x4pro
```

The X4 Pro firmware output is:

```text
.pio/build/x4pro/firmware.bin
```

The repository CI also uploads a dedicated **`firmware-x4pro`** artifact so the X4 Pro binary cannot be confused with the standard X3/X4 build.

## Flashing

Bookshelf firmware is experimental community firmware. Flashing custom firmware is at your own risk.

For an unlocked X4 Pro, the CrossPoint web flasher can be used with its **Custom .bin** option:

[CrossPoint Flash Tools](https://crosspointreader.com/#flash-tools)

Please read CrossPoint's own installation, recovery, and device-locking guidance before flashing. In particular, do not assume instructions for an unlocked device apply to a USB-locked device.

If you do not specifically want the Bookshelf modification, use the official upstream CrossPoint firmware instead.

## What this fork does *not* try to do

Bookshelf is intentionally narrow in scope. It is not trying to add:

- a large library database
- tags or collections
- reading goals or streaks
- social features
- recommendations
- complex statistics
- a replacement reader engine
- a replacement Home/navigation system

The focus is simply **choosing books, seeing what is being read, and reading them**.

## Contributing and upstream CrossPoint

For CrossPoint itself, feature requests, documentation, supported firmware, and upstream development should go to the original project:

**[crosspoint-reader/crosspoint-reader](https://github.com/crosspoint-reader/crosspoint-reader)**

CrossPoint is open-source e-reader firmware maintained by its community of developers and readers. If you enjoy this fork, please also support and contribute to the upstream project that makes it possible.

For Bookshelf-specific experiments or issues, use this fork.

## Credits

**CrossPoint Reader and its contributors** — for the firmware, X4 Pro port, FreeInk-based UI, reader engine, EPUB support, file browser, Recent Books, cover caching, device support, build system, and the foundation this experiment is built on.

Upstream repository: [crosspoint-reader/crosspoint-reader](https://github.com/crosspoint-reader/crosspoint-reader)

CrossPoint website: [crosspointreader.com](https://crosspointreader.com/)

Bookshelf is a small community extension built on top of that work.

---

**CrossPoint Bookshelf is not affiliated with XTEINK or any device manufacturer and is not an official CrossPoint Reader release.**
