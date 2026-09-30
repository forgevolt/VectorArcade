#pragma once

#include "MenuItem.h"
#include "Joystick2Axis.h"

// ---- JoystickCalibration ---------------------------------------------------------------
// Center and min/max calibration for a 2-axis joystick. 

class JoystickCalibration : public MenuItem
{
  public:
    JoystickCalibration(Menu& menu);
    ~JoystickCalibration() override {}

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // From class MenuItem
    void start() override;

  private:
    Joystick2Axis& myJoy;
    
    unsigned long myCalibrationStartTime;
    bool myCenterCalibrationWaitPeriod;    // true: waiting to ensure that joystick is in idle position
    bool myCenterCalibrationIsActive;      // true: center measurement is active
    bool myMinMaxCalibrationIsActive;      // true: min/max measurement is active
    bool myStopCalibration;                // true: user has pressed a button during min/max calibration -> stop
    bool myResultIsActive;                 // true: showing that an axis was not accepted
    unsigned long myResultStartTime;
    Joystick2Axis::CalibrationResult myResult;
    
    int myCount;                           // Number of samples of the center calibration
    long mySumX, mySumY;                   // Sum of all measurements for the center calibration
    int myCenterX, myCenterY;              // Center calibration values
    int myMinX, myMinY;                    // Min values measured
    int myMaxX, myMaxY;                    // Max values measured

    // Square field: range covered so far and current position
    void drawField(int rawX, int rawY);

    // Horizontal (x axis, below the field) and vertical (y axis, left of the field) sliders:
    // range covered so far and current position
    void drawSliderX(int raw);
    void drawSliderY(int raw);

    // Progress bar centered at 'y', 'fraction' in [0, 1]
    void drawProgressBar(int y, float fraction);

    // Lines of text in the column right of the field
    void drawTextLines(const char* const lines[], int numLines);
};