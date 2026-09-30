#include "Board.h"
#if BOARD_HAS_IMU()

#include <Preferences.h>
#include "DisplayObject.h"
#include "IMUCalibration.h"


// ---- IMUCalibration --------------------------------------------------------------------

namespace
{

const unsigned long cWaitDuration  = 5000; // wait time until the calibration begins
const String cMenuName = "IMU Calibration";

} // namespace

// ----------------------------------------------------------------------------------------
IMUCalibration::IMUCalibration(Menu& menu, MPU6050& imu)
: MenuItem(menu, cMenuName),
  myIMU(imu)
{
  loadOffsets();
}

// ----------------------------------------------------------------------------------------
void IMUCalibration::step(unsigned long dt)
{
  if (isActive() == false)
    return;

  // Has the wait time before the calibration already expired?
  if (myIsCalibrationWaitPeriodActive == true && millis()-myCalibrationStartTime > cWaitDuration)
  {
    myIsCalibrationWaitPeriodActive = false;
  }
  else if (myIsCalibrationWaitPeriodActive == false)
  {
    myIMU.calcOffsets(true, true);
    saveOffsets();
    setState(eNotActive);
  }
}

// ----------------------------------------------------------------------------------------
void IMUCalibration::draw()
{
  if (isActive() == false)
  {
    MenuItem::draw();
  }
  else
  {
    // Clear screen
    canvas().setBrushColor(Color::Black);
    canvas().clear();

    canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
    canvas().setPenColor(cDefaultCol);
    canvas().selectFont(&fabgl::FONT_std_24);

    drawCenteredText(30, cMenuName);
    canvas().drawLine(0, 60, canvas().getWidth(), 60);
    drawCenteredText(80, "Place rocket on a");
    drawCenteredText(110, "flat surface!");

    if (myIsCalibrationWaitPeriodActive == true)
    {
      drawCenteredText(160, "Calibration starts in ...");
      String seconds(int(max((float(myCalibrationStartTime)+cWaitDuration-millis())/1000, 0.0f)));
      drawCenteredText(190, seconds);
    }
  }
}

// ----------------------------------------------------------------------------------------
void IMUCalibration::start()
{
  myIsCalibrationWaitPeriodActive = true;
  myCalibrationStartTime = millis();
  setState(eActive);
}

// ----------------------------------------------------------------------------------------
void IMUCalibration::loadOffsets()
{
  // Read calibration data from NVS or use default values
  Preferences p;
  p.begin("IMU");
    myIMU.setGyroOffsets(p.getFloat("gyroXoffset", 0),
                         p.getFloat("gyroYoffset", 0),
                         p.getFloat("gyroZoffset", 0));
    myIMU.setAccOffsets(p.getFloat("accXoffset", 0),
                        p.getFloat("accYoffset", 0),
                        p.getFloat("accZoffset", 0));
  p.end();
}

// ----------------------------------------------------------------------------------------
void IMUCalibration::saveOffsets()
{
  Preferences p;
  p.begin("IMU");
    p.putFloat("gyroXoffset", myIMU.getGyroXoffset());
    p.putFloat("gyroYoffset", myIMU.getGyroYoffset());
    p.putFloat("gyroZoffset", myIMU.getGyroZoffset());
    p.putFloat("accXoffset",  myIMU.getAccXoffset());
    p.putFloat("accYoffset",  myIMU.getAccYoffset());
    p.putFloat("accZoffset",  myIMU.getAccZoffset());
  p.end();
}

#endif
