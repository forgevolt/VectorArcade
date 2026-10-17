#pragma once

#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
#include "DisplayObject.h"

// ---- BatteryStatus ---------------------------------------------------------------------
// Displays an icon with battery status on the screen.

class BatteryStatus : public DisplayObject
{
  public:
    BatteryStatus(fabgl::Canvas& canvas, SFE_MAX1704X& batteryMonitor);
    ~BatteryStatus() override {}

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

  private:
    bool isCharging();

  private:
    // Reference to the MAX17048 I2C battery monitor
    SFE_MAX1704X& myFuelGauge;

    // Access the gauge every 30sec
    unsigned long myLastUpdate;
    float myCellPercent;
    bool myIsCharging;  

    // Charge sense (BOARD_HAS_CHARGE_SENSE): rolling average of the charger's status pin
    unsigned long myNumChargeReadings = 0;
    float myChargeStatValue = 0;
};