#pragma once

#include <Arduino.h>

// ---- Joystick2Axis ---------------------------------------------------------------------
// Representation of the 2-axis analog joystick. The joystick button is not processed 
// within this class.

class Joystick2Axis
{
  public: 
    // Min/max *raw* return values of getRawX/Y methods
    static constexpr int cADC_MIN = 0;
    static constexpr int cADC_MAX = 4095; // ESP32, 12 bit
    
    // Min/max return values of the getX/Y methods
    static constexpr int cMIN = -1000;
    static constexpr int cMAX =  1000;
    
  public:
    Joystick2Axis(int pinX, int pinY, bool invX = false, bool invY = false);

    // Initialize and start processing
    void begin();

    // Calibration information that is taken into account when computing the current value 
    // of an axis (getter methods). An axis whose data leaves no usable travel on both sides
    // of the center falls back to the full ADC range.
    void setCalibrationData(int minX, int maxX, int centerX,
                            int minY, int maxY, int centerY);

    // Reads both axes once and advances the four direction state machines from that reading.
    // Everything below reports it until the next call, so each axis costs one (averaged) ADC
    // read per update.
    void update();

    // The value of the last update(). Range is [-1000, 1000], 0 at the center.
    int getX() const { return myX; }
    int getY() const { return myY; }
 
    // Get the raw values i.e. for calibration purposes. These read the ADC on every call.
    uint16_t getRawX() const { return readRaw(myPinX, myInvX); }
    uint16_t getRawY() const { return readRaw(myPinY, myInvY); }

    // Direction "keys" as of the last update(): true on the edge into a direction and again
    // on each auto-repeat.
    bool isUp()    const { return myIsUp;    }
    bool isDown()  const { return myIsDown;  }
    bool isRight() const { return myIsRight; }
    bool isLeft()  const { return myIsLeft;  }

  private:
    int myPinX, myPinY;
    bool myInvX, myInvY;

    // Calibration, overwritten by begin() and setCalibrationData(). The defaults describe an
    // uncalibrated joystick spanning the full ADC range.
    int myMinX = cADC_MIN, myMaxX = cADC_MAX, myCenterX = cADC_MAX/2;
    int myMinY = cADC_MIN, myMaxY = cADC_MAX, myCenterY = cADC_MAX/2;

    // Values and direction flags of the last update()
    int  myX = 0, myY = 0;
    bool myIsUp = false, myIsDown = false, myIsRight = false, myIsLeft = false;

    // State of one direction "key": on/off thresholds and auto-repeat
    struct DirectionState
    {
      bool myIsActive = false;
      bool myIsRepeatActive = false;
      unsigned long myTimeActive = 0;
      unsigned long myTimeRepeatTriggered = 0;
    };

    DirectionState myUpState, myDownState, myRightState, myLeftState;

    // Averaged ADC reading of one axis, inverted if requested
    uint16_t readRaw(int pin, bool inv) const;

    // Stores one axis's calibration if both halves of the travel clear the dead zone with
    // room left over, otherwise the full-range defaults
    void setAxisCalibration(int& min, int& max, int& center,
                            int newMin, int newMax, int newCenter, const char* axis);

    // Advances the state machine of one direction. 'value' must be signed so that a positive
    // value means "this direction is currently active": down and left pass the negated value.
    static bool updateDirectionState(int value, DirectionState& state);
};
