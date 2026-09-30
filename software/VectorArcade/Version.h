#pragma once

// Firmware version of VectorArcade, MAJOR.MINOR.PATCH, bumped by hand for every release:
//   MAJOR - stored settings (NVS) or the hardware interface change incompatibly
//   MINOR - new feature, e.g. a game, a menu entry or a board
//   PATCH - bug fix
//
// A macro rather than constexpr, so that it can be pasted into #pragma message.
//
// The version names the source; the build timestamp printed at startup (__DATE__/__TIME__)
// names the build. The timestamp is taken when VectorArcade.ino is compiled, so after a
// build that recompiled only other files it can be older than the firmware.

#define VECTORARCADE_VERSION "0.9.0"
