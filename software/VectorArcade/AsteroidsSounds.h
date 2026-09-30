#pragma once

#include "SoundClips.h"

// ---- AsteroidsSounds -------------------------------------------------------------------
// The clips of Asteroids. Owned by Asteroids and handed to Ship, Asteroid and Saucer by
// reference. The constructor is defined in Asteroids.cpp: it loads the clips from LittleFS,
// or links them from sounds/*.h (SOUNDS_FROM_LITTLEFS() 0), which only Asteroids.cpp includes.

struct AsteroidsSounds
{
  explicit AsteroidsSounds(SoundEngine& sound);

  SoundClip rocketEngine;
  SoundClip explosion;
  SoundClip smallExplosion;
  SoundClip pew;
  SoundClip pewSaucer;
  SoundClip saucerAlert;

  private:
    void setVolumes();
};
