#include "DisplayObject.h"
#include <Streaming.h>
#include "JoystickCalibration.h"
#include "MathUtilities.h"


// ---- JoystickCalibration ---------------------------------------------------------------

namespace
{

// Center calibration lasts 6 seconds (3 seconds wait, 3 seconds measurements)
const unsigned long cCenterWaitDuration  = 3000; // wait time until the center measurements begin
const unsigned long cCenterTotalDuration = 6000; 

// Total of 10 seconds to move both axes to their min/max positions
const unsigned long cMinMaxDuration = 10000;

} // namespace

// ----------------------------------------------------------------------------------------
JoystickCalibration::JoystickCalibration(Menu& menu)
: MenuItem(menu, "Joystick Calibration"),
  myJoy(menu.input().joy())
{}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::step(unsigned long dt)
{
  if (isActive() == false)
    return;

  // Check if a button is pressed during execution
  InputController::State s = myMenu.input().getState();

  if (s.a || s.b || s.sel || s.start)  
    myStopCalibration = true;

  // Has the wait time to start the center measurements already expired?
  if (myCenterCalibrationWaitPeriod == true && millis()-myCalibrationStartTime > cCenterWaitDuration)
  {
    myCenterCalibrationWaitPeriod = false;
    myCenterCalibrationIsActive  = true;
  }

  // Center calibration active? Yes -> process center calibration values
  else if (myCenterCalibrationIsActive == true && millis()-myCalibrationStartTime <= cCenterTotalDuration)
  {
    myCount++;
    mySumX += myJoy.getRawX();
    mySumY += myJoy.getRawY();
  }

  // End the center calibration? -> start with min/max calibration
  else if (myCenterCalibrationIsActive == true && millis()-myCalibrationStartTime > cCenterTotalDuration)
  {
    // Unlikely, but better safe than sorry (division by zero)
    if (myCount == 0)
    {
      myCenterCalibrationIsActive = false;
      setState(eNotActive);
      Serial << __PRETTY_FUNCTION__  << " myCount == 0 " << endl;
      return;
    }

    myCenterX = mySumX / myCount;
    myCenterY = mySumY / myCount;

    myMinX = myMaxX = myCenterX;
    myMinY = myMaxY = myCenterY;

    myCenterCalibrationIsActive = false;
    myMinMaxCalibrationIsActive = true;
    myStopCalibration           = false;
    sound().play(sound().signal());
  }

  // User has pressed a button while min/max calibration is active -> abort
  else if (myMinMaxCalibrationIsActive == true && myStopCalibration == true && millis()-myCalibrationStartTime <= cCenterTotalDuration+cMinMaxDuration)
  {
    myStopCalibration = false;
    myCalibrationStartTime += 2*cMinMaxDuration;
  }

  // Min/max calibration: Process current values
  else if (myMinMaxCalibrationIsActive == true && millis()-myCalibrationStartTime <= cCenterTotalDuration+cMinMaxDuration)
  {
    const int x = myJoy.getRawX();
    const int y = myJoy.getRawY();

    myMinX = min(myMinX, x);
    myMaxX = max(myMaxX, x);
    myMinY = min(myMinY, y);
    myMaxY = max(myMaxY, y);
  }

  // End of the min/max calibration?
  else if (myMinMaxCalibrationIsActive == true && millis()-myCalibrationStartTime > cCenterTotalDuration+cMinMaxDuration)
  {
    myMinMaxCalibrationIsActive = false;
    setState(eNotActive);
    myJoy.setCalibrationData(myMinX, myMaxX, myCenterX, myMinY, myMaxY, myCenterY);
  }
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::draw()
{
  if (isActive() == false)
  {
    MenuItem::draw();
    return;
  }

  // Clear screen
  canvas().setBrushColor(Color::Black);
  canvas().clear();

  canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
  canvas().setPenColor(cDefaultCol);
  canvas().selectFont(&fabgl::FONT_std_24);

  if (myCenterCalibrationWaitPeriod == true || myCenterCalibrationIsActive == true)
  {
    drawCenteredText(30, "Center Calibration");
    canvas().setPenColor(cDefaultCol);
    canvas().drawLine(0, 60, canvas().getWidth(), 60);

    if (myCenterCalibrationWaitPeriod == true)
    {
      drawCenteredText(80, "Calibration starts in ...");
      String seconds(int(max((float(myCalibrationStartTime)+cCenterWaitDuration-millis())/1000, 0.0f)));
      drawCenteredText(110, seconds);
    }
    else 
    {
      drawCenteredText(80, "Don't move the joystick!");
    }

    // ----- Progress bar

    canvas().setPenWidth(2);
    canvas().setLineEnds(LineEnds::None);
    canvas().setPenColor(cDefaultCol);
    canvas().drawRectangle(30, 160-10, canvas().getWidth()-30, 160+10);
    canvas().setBrushColor(0, 0, 255);
    canvas().fillRectangle(33, 160-6, 33+float(canvas().getWidth()-66)/cCenterTotalDuration*(millis()-myCalibrationStartTime), 160+7);
  }
  else if (myMinMaxCalibrationIsActive == true)
  {
    drawCenteredText(30, "Min/Max Calibration");
    canvas().setPenColor(cDefaultCol);
    canvas().drawLine(0, 60, canvas().getWidth(), 60);

    drawCenteredText(80, "Move joystick to");
    drawCenteredText(110, "min/max positions.");
    drawCenteredText(140, "Press button to exit.");

    // ----- Progress bar

    canvas().setPenWidth(2);
    canvas().setLineEnds(LineEnds::None);
    canvas().setPenColor(cDefaultCol);
    canvas().drawRectangle(30, 190-10, canvas().getWidth()-30, 190+10);
    canvas().setBrushColor(0, 0, 255);
    canvas().fillRectangle(33, 190-6, 33+min(float(canvas().getWidth()-66)/cMinMaxDuration*(millis()-myCalibrationStartTime-cCenterTotalDuration), 
                                             float(canvas().getWidth()-66)), 190+7);
  } 
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::start()
{
  myCount = 0;
  mySumX = mySumY = 0;
  
  myCenterCalibrationWaitPeriod = true;
  myCenterCalibrationIsActive   = false;
  myMinMaxCalibrationIsActive   = false;
  myStopCalibration             = false;
  myCalibrationStartTime        = millis();

  setState(eActive);
}

