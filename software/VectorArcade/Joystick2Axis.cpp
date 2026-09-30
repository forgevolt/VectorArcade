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

  if (value > centerValue)
    return mapf(value, centerValue + cDeadzone, maxValue, 0, Joystick2Axis::cMAX);
  else
    return mapf(value, minValue, centerValue - cDeadzone, Joystick2Axis::cMIN, 0);
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
  // Read calibration data from NVS or use default values
  Preferences p;
  p.begin("Joystick");
 
  myMinX    = p.getInt("minX",    cADC_MIN);
  myMaxX    = p.getInt("maxX",    cADC_MAX);
  myCenterX = p.getInt("centerX", cADC_MAX/2);

  myMinY    = p.getInt("minY",    cADC_MIN);
  myMaxY    = p.getInt("maxY",    cADC_MAX);
  myCenterY = p.getInt("centerY", cADC_MAX/2);

  p.end();
}

// ----------------------------------------------------------------------------------------
void Joystick2Axis::setCalibrationData(int minX, int maxX, int centerX,
                                       int minY, int maxY, int centerY)
{
  myMinX = minX; myMaxX = maxX; myCenterX = centerX;
  myMinY = minY; myMaxY = maxY; myCenterY = centerY;

  // Store calibration data in NVS
  Preferences p;
  p.begin("Joystick");
 
  p.putInt("minX",    myMinX);
  p.putInt("maxX",    myMaxX);
  p.putInt("centerX", myCenterX);

  p.putInt("minY",    myMinY);
  p.putInt("maxY",    myMaxY);
  p.putInt("centerY", myCenterY);

  p.end();
}

// ----------------------------------------------------------------------------------------
void Joystick2Axis::update()
{
  myX = scaleAxis(getRawX(), myMinX, myCenterX, myMaxX);
  myY = scaleAxis(getRawY(), myMinY, myCenterY, myMaxY);
}

// ----------------------------------------------------------------------------------------
uint16_t Joystick2Axis::getRawX()
{
  uint16_t value = readADC(myPinX);
  return ((myInvX == true) ? cADC_MAX - value : value);
}

// ----------------------------------------------------------------------------------------
uint16_t Joystick2Axis::getRawY()
{
  uint16_t value = readADC(myPinY);
  return ((myInvY == true) ? cADC_MAX - value : value);
}

// ----------------------------------------------------------------------------------------
bool Joystick2Axis::isUp()
{
  int y = getY();

  // Below threshold -> direction "key" is not active
  if (y < cDirOffThreshold)
  {
    myIsUpActive = false;
    myIsUpRepeatActive = false;
    return false;
  }

  // State change from inactive to active? -> direction "key" is active?
  if (myIsUpActive == false && y > cDirOnThreshold)
  {
    myIsUpActive = true;
    myTimeUpActive = millis();
    return true;
  }

  // State change from active to start auto repeat?
  if (myIsUpActive == true && myIsUpRepeatActive == false && millis()-myTimeUpActive > cAutoRepeatDelay)
  {
    myIsUpRepeatActive = true;
    myTimeUpRepeatTriggered = millis();
    return true;
  }

  // Auto repeat already active, trigger another direction "key"?
  if (myIsUpActive == true && myIsUpRepeatActive == true && millis()-myTimeUpRepeatTriggered > cRepeatInterval)
  {
    myTimeUpRepeatTriggered = millis();
    return true;
  }

  return false;
}

// ----------------------------------------------------------------------------------------
bool Joystick2Axis::isDown()
{
  int y = getY();

  // Below threshold -> direction "key" is not active
  if (y > -cDirOffThreshold)
  {
    myIsDownActive = false;
    myIsDownRepeatActive = false;
    return false;
  }

  // State change from inactive to active? -> direction "key" is active?
  if (myIsDownActive == false && y < -cDirOnThreshold)
  {
    myIsDownActive = true;
    myTimeDownActive = millis();
    return true;
  }

  // State change from active to start auto repeat?
  if (myIsDownActive == true && myIsDownRepeatActive == false && millis()-myTimeDownActive > cAutoRepeatDelay)
  {
    myIsDownRepeatActive = true;
    myTimeDownRepeatTriggered = millis();
    return true;
  }

  // Auto repeat already active, trigger another direction "key"?
  if (myIsDownActive == true && myIsDownRepeatActive == true && millis()-myTimeDownRepeatTriggered > cRepeatInterval)
  {
    myTimeDownRepeatTriggered = millis();
    return true;
  }

  return false;
}

// ----------------------------------------------------------------------------------------
bool Joystick2Axis::isRight()
{
  int x = getX();

  // Below threshold -> direction "key" is not active
  if (x < cDirOffThreshold)
  {
    myIsRightActive = false;
    myIsRightRepeatActive = false;
    return false;
  }

  // State change from inactive to active? -> direction "key" is active?
  if (myIsRightActive == false && x > cDirOnThreshold)
  {
    myIsRightActive = true;
    myTimeRightActive = millis();
    return true;
  }

  // State change from active to start auto repeat?
  if (myIsRightActive == true && myIsRightRepeatActive == false && millis()-myTimeRightActive > cAutoRepeatDelay)
  {
    myIsRightRepeatActive = true;
    myTimeRightRepeatTriggered = millis();
    return true;
  }

  // Auto repeat already active, trigger another direction "key"?
  if (myIsRightActive == true && myIsRightRepeatActive == true && millis()-myTimeRightRepeatTriggered > cRepeatInterval)
  {
    myTimeRightRepeatTriggered = millis();
    return true;
  }

  return false;}

// ----------------------------------------------------------------------------------------
bool Joystick2Axis::isLeft()
{
  int x = getX();

  // Below threshold -> direction "key" is not active
  if (x > -cDirOffThreshold)
  {
    myIsLeftActive = false;
    myIsLeftRepeatActive = false;
    return false;
  }

  // State change from inactive to active? -> direction "key" is active?
  if (myIsLeftActive == false && x < -cDirOnThreshold)
  {
    myIsLeftActive = true;
    myTimeLeftActive = millis();
    return true;
  }

  // State change from active to start auto repeat?
  if (myIsLeftActive == true && myIsLeftRepeatActive == false && millis()-myTimeLeftActive > cAutoRepeatDelay)
  {
    myIsLeftRepeatActive = true;
    myTimeLeftRepeatTriggered = millis();
    return true;
  }

  // Auto repeat already active, trigger another direction "key"?
  if (myIsLeftActive == true && myIsLeftRepeatActive == true && millis()-myTimeLeftRepeatTriggered > cRepeatInterval)
  {
    myTimeLeftRepeatTriggered = millis();
    return true;
  }

  return false;
}

