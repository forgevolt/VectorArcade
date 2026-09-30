#pragma once

#include "Adafruit_MAX1704X.h"
#include "DisplayObject.h"

// ---- BatteryStatus ---------------------------------------------------------------------
// Displays an icon with battery status on the screen.

class BatteryStatus : public DisplayObject
{
  public:
    BatteryStatus(fabgl::Canvas& canvas, Adafruit_MAX17048& batteryMonitor);
    ~BatteryStatus() override {}

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

  private:
    bool isCharging();

  private:
    // Reference to the MAX17048 I2C battery monitor
    Adafruit_MAX17048& myFuelGauge;

    // Access the gauge every 30sec
    unsigned long myLastUpdate;
    float myCellPercent;
    bool myIsCharging;  

    // Charge sense (BOARD_HAS_CHARGE_SENSE): rolling average of the charger's status pin
    unsigned long myNumChargeReadings = 0;
    float myChargeStatValue = 0;
};