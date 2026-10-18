#pragma once

#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
#include "DisplayObject.h"

// ---- BatteryStatus ---------------------------------------------------------------------
// Displays an icon with battery status on the screen, and decides when the battery is empty.

class BatteryStatus : public DisplayObject
{
  public:
    BatteryStatus(fabgl::Canvas& canvas, SFE_MAX1704X& batteryMonitor);
    ~BatteryStatus() override {}

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // Called every frame, whatever is on screen: reads the gauge every few seconds and
    // decides whether the battery is empty
    void monitor();

    // Empty: the gauge has shown 0 % - or the voltage has stayed below a safe minimum - for
    // a minute. It stays empty until the gauge shows a few percent again, while charging.
    bool isEmpty() const { return myIsEmpty; }

    // Full screen in place of the menu while the battery is empty
    void drawEmpty();

  private:
    bool isCharging();

  private:
    // Reference to the MAX17048 I2C battery monitor
    SFE_MAX1704X& myFuelGauge;

    bool myHasGauge;
    unsigned long myLastReading;  // the gauge is read every cReadInterval
    float myCellPercent;
    bool myIsCharging;  
    bool myIsEmpty = false;
    unsigned long myLowSince = 0; // since when the empty condition holds, 0: it does not

    // Charge sense (BOARD_HAS_CHARGE_SENSE): rolling average of the charger's status pin
    unsigned long myNumChargeReadings = 0;
    float myChargeStatValue = 0;
};