#pragma once

#include "SoundClips.h"

// ---- LunarLanderSounds -----------------------------------------------------------------
// The clips of Lunar Lander. Owned by LunarLander and handed to the Eagle by reference.
// The constructor is defined in LunarLander.cpp: it loads the clips from LittleFS, or links
// them from sounds/*.h (SOUNDS_FROM_LITTLEFS() 0), which only LunarLander.cpp includes.

struct LunarLanderSounds
{
  explicit LunarLanderSounds(SoundEngine& sound);

  SoundClip rocketEngine;
  SoundClip fuelAlarm;
  SoundClip explosion;

  private:
    void setVolumes();
};
