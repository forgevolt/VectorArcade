#include "Board.h"
#if BOARD_HAS_IMU()

#include "IMUStatus.h"
#include "MathUtilities.h"

// ---- IMUStatus -------------------------------------------------------------------------

namespace
{

const int   cRadius     = 12;    // Radius of the circle 
const float cAngleLimit = 45;    // Visualize in [-cAngleLimit, cAngleLimit]
const int   cPosXTemp   = 90;    // Position of the temperature
const int   cPosYTemp   = 3;
const int   cPosXIMU    = 210;   // Position of the IMU information
const int   cPosYIMU    = 17;

} // namespace

// ----------------------------------------------------------------------------------------
IMUStatus::IMUStatus(fabgl::Canvas& canvas, MPU6050& imu)
: DisplayObject(canvas),
  myIMU(imu),
  myLastTempUpdate(millis()),
  myTemp(-1)
{}

// ----------------------------------------------------------------------------------------
void IMUStatus::step(unsigned long dt)
{
  if (isActive() == false)
    return;
  
  // ---- IMU
  // Fast updates (no delays!)
  myIMU.update();

  // Update temperature every 15sec to avoid value "flickering"
  if (millis() - myLastTempUpdate > 15000 || myTemp < 0)
  { 
    myLastTempUpdate = millis();
    myTemp = myIMU.getTemp();
  }
}

// ----------------------------------------------------------------------------------------
void IMUStatus::draw()
{
  if (isActive() == false)
    return;

  // Note: IMU getters do not initiate a call to the hardware sensor. A cached value is returned instead.
  float angleX = myIMU.getAngleX();
  float angleY = myIMU.getAngleY();

  // Temperature as measured by the MPU-6050
  canvas().setPenColor(cDefaultCol);
  canvas().selectFont(&fabgl::FONT_std_24);
  canvas().drawTextFmt(cPosXTemp, cPosYTemp, " %2.1f ", (double)myTemp);
  canvas().selectFont(&fabgl::FONT_9x18);
  canvas().drawText(cPosXTemp+50, cPosYTemp-5, "o");

  // Visualize angles
  angleX = constrain(angleX, -cAngleLimit, +cAngleLimit);
  angleY = constrain(angleY, -cAngleLimit, +cAngleLimit);

  // Map the square [-1, 1] x [-1, 1] to the unit disk so that the spot stays inside the circle
  Vector2 spot = circularNormalization(angleY / cAngleLimit, angleX / cAngleLimit) * (cRadius-3);
  
  canvas().drawLine(cPosXIMU-cRadius, cPosYIMU, cPosXIMU+cRadius, cPosYIMU);
  canvas().drawLine(cPosXIMU, cPosYIMU-cRadius, cPosXIMU, cPosYIMU+cRadius);
  canvas().drawEllipse(cPosXIMU, cPosYIMU, cRadius*2, cRadius*2);
  canvas().setBrushColor(71,  212, 90);
  canvas().fillEllipse(spot.y*-1+cPosXIMU, spot.x+cPosYIMU, 5, 5);
}

#endif
