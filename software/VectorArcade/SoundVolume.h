#pragma once

#include "MenuItem.h"

// ---- SoundVolume -----------------------------------------------------------------------
// Set global sound volume

class SoundVolume : public MenuItem
{
  public:
    SoundVolume(Menu& menu);
    ~SoundVolume() override {};

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // From class MenuItem
    void start()   override;

  private:
    void loadVolume();
    void saveVolume();
};