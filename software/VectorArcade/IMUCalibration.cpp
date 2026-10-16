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

  // Read the buttons on every step, so that no press is left over for the menu. Sel leaves
  // during the wait, before anything is measured: the stored offsets stay as they are.
  InputController::State s = myMenu.input().getState();

  if (s.sel == true && myIsCalibrationWaitPeriodActive == true)
  {
    sound().play(sound().click());
    setState(eNotActive);
    return;
  }

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

    drawHeader(30, cMenuName);
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
