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
    bool myCenterCalibrationIsActive;      // true: center calibration (the releases) is active
    int  myRelease;                        // Center calibration: current release, 0 .. cNumReleases-1

    // The current release: pushed in the step's direction, let go, holding still, measured
    enum class ERelease { ePush, eLetGo, eSettle, eMeasure, eDone };
    ERelease myReleaseState;
    unsigned long myReleaseStateTime;      // start of the still window, the measurement or the check mark
    std::array<int, 2> myStillMin;         // raw range of each axis within the still window
    std::array<int, 2> myStillMax;
    std::array<long, 2> myRestSum;         // rest positions of the finished releases
    bool myMinMaxCalibrationIsActive;      // true: min/max measurement is active
    bool myStopCalibration;                // true: user has pressed a button during min/max calibration -> stop
    bool myResultIsActive;                 // true: showing that not every edge was reached
    unsigned long myResultStartTime;
    
    joystickutilities::CenterSampler<2> myCenter;  // Center calibration: rest position of the current release
    joystickutilities::RangeTracker<2>  myRange;   // Min/max calibration: center and travel of each axis

    // Advances the current release; starts the min/max calibration after the last one
    void stepRelease();

    // Widens the still window by one raw reading. When an axis has moved further than
    // cStillCounts, the window starts again from this reading and the result is false.
    bool isStill();

    // Center from the rest positions of all releases; hands over to the min/max calibration
    void startMinMax();

    // The joystick as a circle: an arrow in the step's direction in the color of its state (a
    // check mark once measured), a dot where the stick is, the state in words below and one
    // progress dot per release
    void drawStick();

    // The min/max calibration in the circle of drawStick(): the range covered so far, one marker
    // per edge (filled once reached; on the result also those not reached, in their own color),
    // a dot where the stick is (not on the result) and the edges reached in words below
    void drawRange(bool result);

    // Bar below the circle with the time left in the min/max calibration, 'fraction' in [0, 1]
    void drawTimeLeft(float fraction);
};