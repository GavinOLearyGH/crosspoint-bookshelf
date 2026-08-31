# TIP X4 Companion — Phase 0 Technical Baseline

## Purpose

This document records the technical baseline for adding a lightweight The Irish Par (TIP) golf companion to the XTEINK X4 Pro while preserving the existing CrossPoint Reader and Bookshelf experience.

No TIP feature code is introduced in Phase 0. The goal is to determine what the current hardware/firmware can support safely and which existing CrossPoint patterns TIP should reuse.

## Authoritative starting point

- Repository: `GavinOLearyGH/crosspoint-bookshelf`
- TIP Phase 0 branch: `tip-companion-phase0`
- Parent branch: `feature/bookshelf-v1.1`
- Parent head at Phase 0 start: `1303db95c6dd53f820bb292a13b07aa9512ff2b9`
- Bookshelf upstream baseline documented by the project: `b4a240161016ffbc0d8ca4f8917afe870b6c2594`
- Target: XTEINK X4 Pro

Important: the repository default branch `develop` has continued to move with upstream CrossPoint work. TIP development should not begin from moving `develop` until its X4 Pro/Bookshelf changes are deliberately reconciled. The physically-tested Bookshelf lineage is the safer starting point for the first TIP proof of concept.

## Hardware baseline

The X4 Pro PlatformIO target uses:

- ESP32-S3
- 16 MB flash
- 8 MB PSRAM
- SSD1677-family e-ink display support
- GT911 capacitive touch
- dual warm/cold frontlight support
- native SDMMC storage
- Wi-Fi via the existing CrossPoint/Arduino stack

The build defines `FREEINK_DEVICE_X4PRO`, `BOARD_HAS_PSRAM`, and uses the `x4pro` PlatformIO environment.

### Implication for TIP

The X4 Pro has materially more memory headroom than the original C3-based X3/X4 targets, but TIP should still be designed as a small state-machine application rather than a rich application platform. The existing firmware already contains reader, networking, TLS, image and EPUB workloads; TIP should avoid creating a second heavy runtime.

## Existing application architecture

CrossPoint uses an Activity-based navigation architecture.

`ActivityManager` owns the current activity, supports push/pop/replace behavior, handles the activity stack, routes Home gestures, and delegates rendering to a dedicated FreeRTOS render task.

Existing functional areas are already separated into activities including:

- Home
- File Browser
- Recent Books
- Reader
- Network / Wi-Fi
- Settings
- Boot / Sleep
- OPDS

This separation is favorable for TIP.

### Recommended TIP integration pattern

TIP should be added as a new isolated activity family, conceptually:

```text
src/activities/tip/
    TipHomeActivity
    TipPocketRefActivity
    TipPracticeActivity
    TipGolferActivity
    TipPlayActivity
```

The reader engine should remain untouched unless a shared integration point is unavoidable.

## Reading/Bookshelf protection rule

The existing Bookshelf project explicitly follows the principle:

> CrossPoint remains the firmware; Bookshelf is an add-on.

TIP should extend that rule:

> CrossPoint remains the reader; Bookshelf remains the reading enhancement; TIP Companion is a second isolated enhancement.

The firmware should continue to expose normal reading features, including Bookshelf, Browse/Library, Recent Books, OPDS where configured, File Transfer, Settings, and the existing EPUB reader.

A TIP failure must not prevent the device from being used as an e-reader.

## Sleep and wake behavior

This is the most important Phase 0 finding.

The X4 Pro does not preserve a live application process across sleep in the way a phone suspends an app. CrossPoint's sleep path saves state, tears down Wi-Fi, puts the display and peripherals to sleep, and enters the hardware sleep/power path. Wake is treated as a new boot/reset path.

CrossPoint already implements a user-perceived fast-resume pattern:

1. Save application state to SD.
2. Optionally save/retain the visible frame.
3. Suppress the normal boot splash on the next wake.
4. Reinitialize hardware and storage.
5. Load persisted application state.
6. Route directly to the appropriate activity.
7. Use the retained e-ink frame to mask the reboot until the useful screen is painted.

For reading, CrossPoint persists `openEpubPath`, tracks whether sleep occurred from the reader, and routes directly back into `ReaderActivity` when appropriate.

### TIP fast-resume design

TIP should reuse this exact concept rather than attempting RAM persistence.

Before sleep, TIP should persist a tiny resume record such as:

```json
{
  "activity": "practice",
  "sessionId": "2026-08-29-approach-01",
  "screen": "result-entry",
  "shot": 11
}
```

or:

```json
{
  "activity": "play",
  "roundId": "2026-08-29-springhaven",
  "hole": 7,
  "stage": "approach"
}
```

On wake, boot routing can restore the appropriate TIP activity directly.

### Phase 1 acceptance requirement

A TIP activity must be able to:

- save state,
- sleep,
- wake,
- bypass unnecessary launcher navigation,
- and restore the same logical screen/state.

This should be tested on physical X4 Pro hardware; the simulator cannot reproduce the true hardware deep-sleep path exactly.

## Persistent storage

CrossPoint already uses small JSON-backed persistent stores on the SD card.

Examples include:

- `/.crosspoint/state.json`
- Bookshelf's `/.crosspoint/bookshelf.json`

This is an excellent fit for TIP.

### Recommended TIP storage root

```text
/.crosspoint/tip/
    state.json
    golfer.json
    bag.json
    yardages.json
    wedge_matrix.json
    current_plan.json
    rounds/
    sessions/
    refs/
```

TIP should favor small independent files rather than one large database. This improves recoverability, keeps parsing simple, and allows individual sync domains later.

## Networking baseline

The firmware already has reusable network infrastructure:

- Wi-Fi selection and saved-network handling
- network activities
- HTTPS/TLS support through the existing SecureNet/wolfSSL stack
- WebSocket dependency
- OPDS networking
- Calibre connection support
- CrossPoint web/file-transfer server
- KOReader progress synchronization

This means a TIP sync client is technically realistic without adding a new networking stack.

## KOReader/KOSync finding

CrossPoint already contains a `KOReaderSyncActivity` and `KOReaderSyncClient` flow.

Its current model is broadly:

1. connect to Wi-Fi if necessary,
2. identify the document,
3. fetch remote state,
4. compare remote/local progress,
5. apply or upload progress,
6. release Wi-Fi / return to reading.

The activity explicitly prevents auto-sleep while connecting, syncing or uploading.

### Recommendation

Do **not** encode golfer state into the KOReader synchronization protocol.

Instead, reuse the engineering patterns:

- saved Wi-Fi selection,
- short-lived connectivity,
- HTTPS/TLS client setup,
- state machine for CONNECTING/SYNCING/SUCCESS/FAIL,
- explicit auto-sleep suppression during transfer,
- Wi-Fi teardown after network work.

TIP should ultimately use its own small HTTP/HTTPS API and data contract.

## Connectivity policy for TIP

TIP should be offline-first.

Recommended lifecycle:

```text
Wake / Use device
        |
        v
Local TIP state only
        |
        v
User requests Sync
(or a later safe boundary)
        |
        v
Enable Wi-Fi
        |
        v
Upload new round/session records
        |
        v
Download golfer/plan/reference updates
        |
        v
Persist to SD
        |
        v
Disable Wi-Fi
```

Do not keep Wi-Fi persistently active during a round or range session.

## Power considerations

The firmware already:

- reduces CPU activity during idle periods,
- detects active/background work to inhibit sleep,
- disables Wi-Fi before sleep,
- uses retained e-ink content to reduce unnecessary redraw behavior,
- has configurable inactivity sleep behavior.

There have also been upstream/community reports of X4 Pro sleep-drain issues on some beta builds. TIP therefore must not assume all observed battery behavior is caused by TIP itself.

### TIP power guardrails

- No continuous polling.
- No background network loop during golf.
- No unnecessary screen refreshes.
- Persist only at meaningful boundaries or debounce frequent writes.
- Networking must always have timeout/error exits.
- All sync paths must ultimately return Wi-Fi to the normal off/idle state.

## E-ink rendering baseline

CrossPoint supports multiple refresh modes and already treats e-ink refresh behavior carefully.

The sleep renderer deliberately uses a single HALF refresh instead of repeated full/GC flashes. The main application also supports faster initial paints in selected resume flows.

### TIP rendering policy

TIP should prefer:

- static high-contrast screens,
- large text,
- limited animation,
- screen-level state changes,
- minimal unnecessary refreshes,
- fast/partial refresh only where the existing display abstraction safely supports it.

A drill timer should not attempt smartphone-like frame-by-frame updates. Update at meaningful intervals or use a coarse countdown representation.

## Input baseline

The X4 Pro target has:

- physical button input through `MappedInputManager`,
- capacitive touch support,
- a Home gesture/key path,
- power-button sleep handling.

`ActivityManager` already provides global Home routing and activity-level override hooks.

### TIP input recommendation

The first TIP build should work fully with physical controls. Touch can enhance it, but should not be required for core Play/Practice flows.

This keeps interaction reliable in a golf environment and fits the proposed custom case concept.

## Timers and randomization

The firmware already uses Arduino timing (`millis`) and the platform random function. Both are sufficient for:

- warm-up timers,
- practice-block timers,
- random yardage generation,
- challenge counters,
- simple session timing.

No additional framework is required.

## QR capability

The build already depends on `ricmoo/QRCode`.

Therefore later QR handoff screens are technically feasible without adding another QR library. This could support links such as "Open full review in TIP OS" or "Open this golfer profile".

QR is not required for the first TIP proof of concept.

## Memory / performance position

The X4 Pro environment provides 8 MB PSRAM and 16 MB flash, but the firmware already includes substantial reader and network functionality.

The codebase also contains explicit heap-optimization work for TLS and low-memory reader sessions, which is a signal that network operations should remain tightly scoped even on capable targets.

### Phase 0 conclusion on headroom

There is sufficient architectural headroom for a lightweight TIP companion based on:

- Activity screens,
- small JSON stores,
- simple state machines,
- timers/randomization,
- short-lived HTTPS sync,
- static Pocket Ref content.

There is **not** a good reason to run AI inference, GPS mapping, a large analytics engine, a browser-class TIP OS client, or continuously connected services on the X4 Pro.

## Proposed top-level architecture

```text
CrossPoint Firmware
|
+-- Reading
|   +-- Home / Recent
|   +-- Bookshelf
|   +-- Browse / Library
|   +-- Reader
|   +-- OPDS
|
+-- TIP Companion
    +-- Today
    +-- Play
    +-- Practice
    +-- Pocket Ref
    +-- Golfer
    +-- Sync
```

Cloud responsibilities remain outside the X4:

```text
TIP OS / TIP Cloud
    |
    +-- golfer model
    +-- coaching intelligence
    +-- recommendations
    +-- journal / history
    +-- analytics
    +-- plan generation
           |
           | small sync payloads
           v
      TIP X4 Companion
    execution + capture
```

## Phase 0 decisions

### GO

- TIP as an isolated activity family: **GO**
- Pocket Ref: **GO**
- Local golfer/bag/yardage JSON: **GO**
- Random-yardage practice engine: **GO**
- Timed warm-up/practice flows: **GO**
- Persisted round/practice state: **GO**
- User-perceived fast resume through persisted state + splashless wake: **GO**
- Short-lived HTTPS sync to TIP OS/cloud: **GO**
- QR handoff: **GO later**
- Preserve Bookshelf/Library/Reader: **MANDATORY**

### DO NOT BUILD ON X4

- On-device AI/LLM coaching
- full TIP OS web client
- continuously connected cloud session
- heavy analytics/strokes-gained engine
- GPS mapping as part of the initial architecture
- animated/touch-heavy smartphone UI

## Remaining physical-device measurements

Static code review can establish architecture, but these values require a physical X4 Pro test build and instrumentation:

1. cold wake-to-first-useful-screen time,
2. splashless wake-to-first-useful-screen time,
3. TIP state restore time once implemented,
4. free heap / max allocation before and after a TIP activity,
5. free heap during HTTPS sync,
6. battery impact of repeated golf-style wake/use/sleep cycles,
7. e-ink ghosting after repeated result-entry updates,
8. custom-case button accessibility and wake behavior.

These are Phase 1 validation measurements, not blockers to the Phase 0 architectural decision.

## Phase 0 exit decision

**PASS.**

The X4 Pro and current CrossPoint architecture are suitable for a TIP golf companion provided the implementation remains deliberately lightweight, offline-first, and isolated from the reader engine.

The strongest existing patterns to reuse are:

1. Activity-based feature isolation.
2. SD-backed JSON persistence.
3. CrossPoint's splashless/retained-frame wake strategy.
4. KOReader Sync's short-lived networking lifecycle.
5. Existing e-ink refresh abstractions.
6. Existing physical-button/touch input abstraction.

## Recommended Phase 1

Build only the TIP shell and Pocket Ref integration first.

The first engineering branch should prove:

```text
CrossPoint Home
   |
   +-- Bookshelf / Browse / Read (unchanged)
   |
   +-- The Irish Par
          |
          +-- Today
          +-- Practice
          +-- Pocket Ref
          +-- Golfer
          +-- Play (placeholder)
```

Do not implement cloud sync or full round tracking in the first code milestone.
