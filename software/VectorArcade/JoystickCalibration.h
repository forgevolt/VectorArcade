#pragma once

#include "MenuItem.h"
#include <JoystickUtilities.h>

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
    joystickutilities::Joystick<2>& myJoy;
    
    unsigned long myCalibrationStartTime;
    bool myCenterCalibrationWaitPeriod;    // true: waiting to ensure that joystick is in idle position
    bool myCenterCalibrationIsActive;      // true: center measurement is active
    bool myMinMaxCalibrationIsActive;      // true: min/max measurement is active
    bool myStopCalibration;                // true: user has pressed a button during min/max calibration -> stop
    bool myResultIsActive;                 // true: showing that an axis was not accepted
    unsigned long myResultStartTime;
    joystickutilities::CalibrationResult<2> myResult;
    
    joystickutilities::CenterSampler<2> myCenter;  // Center calibration: averaged resting position
    joystickutilities::RangeTracker<2>  myRange;   // Min/max calibration: center and travel of each axis

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