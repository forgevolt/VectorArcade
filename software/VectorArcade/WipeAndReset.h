#pragma once

#include "MenuItem.h"

// ---- WipeAndReset ----------------------------------------------------------------------
// Clears all calibration data and other configuration stored in NVS.

class WipeAndReset : public MenuItem
{
  public:
    WipeAndReset(Menu& menu);
    ~WipeAndReset() override {};

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // From class MenuItem
    void start()   override;

  private:
    bool myIsYesSelected;
};