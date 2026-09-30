#pragma once

#include <SoundEngine.h>
#include "Board.h"

// ---- SoundClip -------------------------------------------------------------------------
// The clip type of the games' sounds, selected by SOUNDS_FROM_LITTLEFS() in BoardSelect.h:
// a FileClip loaded from LittleFS into PSRAM, or an AudioClip on the data linked in from
// sounds/*.h.

#ifndef SOUNDS_FROM_LITTLEFS
  #error "BoardSelect.h: SOUNDS_FROM_LITTLEFS() is missing, see BoardSelect.h.template"
#endif

#if SOUNDS_FROM_LITTLEFS()
  #if !defined(BOARD_HAS_PSRAM)
    #error "SOUNDS_FROM_LITTLEFS() needs PSRAM: enable it in Tools > PSRAM, or set SOUNDS_FROM_LITTLEFS() to 0 in BoardSelect.h"
  #endif

  using SoundClip = FileClip;
  #define SOUNDS_SOURCE "from LittleFS"

  // Loads a WAV file from LittleFS (mounted by setup()) and reports the result on Serial.
  // Blocking: call at start-up only.
  bool loadSoundClip(SoundEngine& sound, FileClip& clip, const char* path);

  // Compares /sounds.id on LittleFS with the fingerprint compiled into the firmware (both
  // written by software/tools/wav2h.py) and reports a mismatch on Serial. Returns true if
  // the WAV files on LittleFS are the ones this firmware was built with.
  bool checkSoundFiles();
#else
  using SoundClip = AudioClip;
  #define SOUNDS_SOURCE "linked into the firmware"
#endif
