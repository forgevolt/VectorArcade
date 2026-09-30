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
    // of an axis (getter methods).
    void setCalibrationData(int minX, int maxX, int centerX,
                            int minY, int maxY, int centerY);

    // Read both axes once. getX/getY and isUp/isDown/isRight/isLeft evaluate the values of
    // the last update(), so each axis costs one (averaged) ADC read per update.
    void update();

    // Get the current value of one of the axes. Range is [-1000, 1000]. Number 0 indicates 
    // the center position. This is also the default that is returned, if an invalid pin 
    // number is provided with the ctor.
    int getX() const { return myX; }
    int getY() const { return myY; }
 
    // Get the raw values i.e. for calibration purposes. These read the ADC on every call.
    uint16_t getRawX();
    uint16_t getRawY();

    // Check if joystick is pressed in one of the four directions. Returns true if direction is active.
    bool isUp();
		bool isDown();
		bool isRight();
		bool isLeft();

  private:
    int myPinX, myPinY;
    bool myInvX, myInvY;
    int myMinX, myMaxX, myCenterX;
    int myMinY, myMaxY, myCenterY;
    int myX = 0, myY = 0; // Values of the last update()

    // "Up" direction
    bool myIsUpActive = false;
    bool myIsUpRepeatActive = false;
    unsigned long myTimeUpActive;
    unsigned long myTimeUpRepeatTriggered;

    // "Down" direction
    bool myIsDownActive = false;
    bool myIsDownRepeatActive = false;
    unsigned long myTimeDownActive;
    unsigned long myTimeDownRepeatTriggered;

    // "Right" direction
    bool myIsRightActive = false;
    bool myIsRightRepeatActive = false;
    unsigned long myTimeRightActive;
    unsigned long myTimeRightRepeatTriggered;

    // "Left" direction
    bool myIsLeftActive = false;
    bool myIsLeftRepeatActive = false;
    unsigned long myTimeLeftActive;
    unsigned long myTimeLeftRepeatTriggered;
};
