#include <Preferences.h>
#include <Streaming.h>
#include "Joystick2Axis.h"
#include "MathUtilities.h"

// ---- Joystick2Axis ---------------------------------------------------------------------

namespace
{

// cDeadzone is the distance that needs to be moved before it interacts
const int cDeadzone = 50;     

// Threshold values to trigger up, down, left and right 
const int cDirOnThreshold = 500;
const int cDirOffThreshold = 300;

// Auto repeat related constants
const unsigned long cAutoRepeatDelay = 1500;
const unsigned long cRepeatInterval  = 300;

// Number of ADC readings averaged per axis to reduce the ESP32 ADC noise
const int cNumSamples = 4;

// ----------------------------------------------------------------------------------------
uint16_t readADC(int pin)
{
  uint32_t sum = 0;
  for (int i=0; i<cNumSamples; i++)
    sum += analogRead(pin);
  return sum / cNumSamples;
}

// ----------------------------------------------------------------------------------------
// Maps a raw ADC value to [cMIN, cMAX] with a dead zone around the calibrated center
int scaleAxis(int value, int minValue, int centerValue, int maxValue)
{
  if (value >= centerValue - cDeadzone && value <= centerValue + cDeadzone)
    return 0;

  // We must not exceed min/max values from the calibration process
  value = (value > maxValue) ? maxValue : value;
  value = (value < minValue) ? minValue : value;

  const float mapped = (value > centerValue)
                     ? mapf(value, centerValue + cDeadzone, maxValue, 0, Joystick2Axis::cMAX)
                     : mapf(value, minValue, centerValue - cDeadzone, Joystick2Axis::cMIN, 0);

  // The getters promise [cMIN, cMAX] for any calibration data
  return constrain(int(mapped), Joystick2Axis::cMIN, Joystick2Axis::cMAX);
}

} // namespace

// ----------------------------------------------------------------------------------------
Joystick2Axis::Joystick2Axis(int pinX, int pinY, bool invX, bool invY)
: myPinX(pinX), myPinY(pinY),
  myInvX(invX), myInvY(invY)
{}

// ----------------------------------------------------------------------------------------
void Joystick2Axis::begin()
{
  // Read calibration data from NVS, or keep the defaults. Opened read-only: a console that
  // has never been calibrated has no "Joystick" namespace, and a read-write open would create
  // an empty one. getInt() then returns the defaults passed below.
  Preferences p;
  p.begin("Joystick", true);

  setAxisCalibration(myMinX, myMaxX, myCenterX,
                     p.getInt("minX",    cADC_MIN),
                     p.getInt("maxX",    cADC_MAX),
                     p.getInt("centerX", cADC_MAX/2), "X");

  setAxisCalibration(myMinY, myMaxY, myCenterY,
                     p.getInt("minY",    cADC_MIN),
                     p.getInt("maxY",    cADC_MAX),
                     p.getInt("centerY", cADC_MAX/2), "Y");

  p.end();
}

// ----------------------------------------------------------------------------------------
Joystick2Axis::CalibrationResult Joystick2Axis::setCalibrationData(int minX, int maxX, int centerX,
                                                                   int minY, int maxY, int centerY)
{
  CalibrationResult result;
  result.x = setAxisCalibration(myMinX, myMaxX, myCenterX, minX, maxX, centerX, "X");
  result.y = setAxisCalibration(myMinY, myMaxY, myCenterY, minY, maxY, centerY, "Y");

  // Store calibration data in NVS. The members rather than the parameters, so a rejected
  // axis stores the defaults it fell back to.
  Preferences p;
  p.begin("Joystick");
 
  p.putInt("minX",    myMinX);
  p.putInt("maxX",    myMaxX);
  p.putInt("centerX", myCenterX);

  p.putInt("minY",    myMinY);
  p.putInt("maxY",    myMaxY);
  p.putInt("centerY", myCenterY);

  p.end();

  return result;
}

// ----------------------------------------------------------------------------------------
bool Joystick2Axis::setAxisCalibration(int& min, int& max, int& center,
                                       int newMin, int newMax, int newCenter, const char* axis)
{
  // Both halves of the travel have to clear the dead zone with room left over. Below that
  // the mapping divides by a span that is zero or negative, and the axis is stuck at full
  // deflection rather than merely inaccurate. An axis that was not moved during the min/max
  // calibration arrives here with all three values equal.
  if (newMin + 2*cDeadzone < newCenter && newCenter + 2*cDeadzone < newMax)
  {
    min    = newMin;
    max    = newMax;
    center = newCenter;
    return true;
  }

  min    = cADC_MIN;
  max    = cADC_MAX;
  center = cADC_MAX/2;

  Serial << "Joystick: axis " << axis << ": min " << newMin << " max " << newMax
         << " center " << newCenter << " leaves no usable travel - using the full ADC range"
         << endl;
  return false;
}

// ----------------------------------------------------------------------------------------
void Joystick2Axis::update()
{
  myX = scaleAxis(getRawX(), myMinX, myCenterX, myMaxX);
  myY = scaleAxis(getRawY(), myMinY, myCenterY, myMaxY);

  // All four run on every update: the state machine keeps timestamps, so a direction that
  // was skipped could fire an immediate auto-repeat when it is evaluated next.
  myIsUp    = updateDirectionState( myY, myUpState);
  myIsDown  = updateDirectionState(-myY, myDownState);
  myIsRight = updateDirectionState( myX, myRightState);
  myIsLeft  = updateDirectionState(-myX, myLeftState);
}

// ----------------------------------------------------------------------------------------
uint16_t Joystick2Axis::readRaw(int pin, bool inv) const
{
  uint16_t value = readADC(pin);
  return ((inv == true) ? cADC_MAX - value : value);
}

// ----------------------------------------------------------------------------------------
bool Joystick2Axis::updateDirectionState(int value, DirectionState& state)
{
  // Below threshold -> direction "key" is not active
  if (value < cDirOffThreshold)
  {
    state.myIsActive = false;
    state.myIsRepeatActive = false;
    return false;
  }

  // State change from inactive to active? -> direction "key" is active?
  if (state.myIsActive == false && value > cDirOnThreshold)
  {
    state.myIsActive = true;
    state.myTimeActive = millis();
    return true;
  }

  // State change from active to start auto repeat?
  if (state.myIsActive == true && state.myIsRepeatActive == false && millis()-state.myTimeActive > cAutoRepeatDelay)
  {
    state.myIsRepeatActive = true;
    state.myTimeRepeatTriggered = millis();
    return true;
  }

  // Auto repeat already active, trigger another direction "key"?
  if (state.myIsActive == true && state.myIsRepeatActive == true && millis()-state.myTimeRepeatTriggered > cRepeatInterval)
  {
    state.myTimeRepeatTriggered = millis();
    return true;
  }

  return false;
}
