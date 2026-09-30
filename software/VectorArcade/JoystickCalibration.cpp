#include "DisplayObject.h"
#include <Streaming.h>
#include "JoystickCalibration.h"
#include "Layout.h"
#include "MathUtilities.h"


// ---- JoystickCalibration ---------------------------------------------------------------

namespace
{

// Center calibration lasts 6 seconds (3 seconds wait, 3 seconds measurements)
const unsigned long cCenterWaitDuration  = 3000; // wait time until the center measurements begin
const unsigned long cCenterTotalDuration = 6000; 

// Total of 10 seconds to move both axes to their min/max positions
const unsigned long cMinMaxDuration = 10000;

// Time an axis that was not accepted is reported on screen, unless a button is pressed
const unsigned long cResultDuration = 5000;

// Geometry around the position field (pixels)
const int cSliderWidth = 12;   // Width of the x/y sliders
const int cSliderGap   = 6;    // Distance between field and slider
const int cSliderInset = 3;    // Gap between a slider's frame and its blue bar, as in the progress bar
const int cDotSize     = 9;    // Diameter of the current-position dot
const int cSliderDot   = 7;    // Diameter of the dot on a slider

// ----------------------------------------------------------------------------------------
// Position of a raw ADC value on an axis that is 'len' pixels long, with the calibrated
// center in the middle: [cADC_MIN, center] fills the first half, [center, cADC_MAX] the
// second, so the middle always marks the center and each half shows its own travel.
int axisPos(int raw, int center, int len)
{
  if (raw <= center)
    return mapf(raw, Joystick2Axis::cADC_MIN, center, 0, len/2);

  return mapf(raw, center, Joystick2Axis::cADC_MAX, len/2, len);
}

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

  // Reporting an axis that was not accepted? -> leave after a while or on a button
  if (myResultIsActive == true)
  {
    if (myStopCalibration == true || millis()-myResultStartTime > cResultDuration)
    {
      myResultIsActive = false;
      setState(eNotActive);
    }
    return;
  }

  // Has the wait time to start the center measurements already expired?
  if (myCenterCalibrationWaitPeriod == true && millis()-myCalibrationStartTime > cCenterWaitDuration)
  {
    myCenterCalibrationWaitPeriod = false;
    myCenterCalibrationIsActive  = true;
    sound().play(sound().signal());
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
    myResult = myJoy.setCalibrationData(myMinX, myMaxX, myCenterX, myMinY, myMaxY, myCenterY);

    if (myResult.x == true && myResult.y == true)
    {
      setState(eNotActive);
    }
    else
    {
      myResultIsActive  = true;
      myResultStartTime = millis();
      myStopCalibration = false;
    }
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

  bool isCenterPhase = (myCenterCalibrationWaitPeriod == true || myCenterCalibrationIsActive == true);
  drawCenteredText(30, isCenterPhase ? "Center Calibration" : "Min/Max Calibration");
  canvas().setPenColor(cDefaultCol);
  canvas().drawLine(0, 60, canvas().getWidth(), 60);

  // One reading per frame for the dots
  int rawX = myJoy.getRawX();
  int rawY = myJoy.getRawY();

  if (isCenterPhase == true)
  {
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

    drawProgressBar(160, float(millis()-myCalibrationStartTime) / cCenterTotalDuration);
  }
  else if (myMinMaxCalibrationIsActive == true)
  {
    drawField(rawX, rawY);
    drawSliderX(rawX);
    drawSliderY(rawY);

    const char* const lines[] = { "Move the", "joystick to", "all edges.", "", "A button", "exits." };
    drawTextLines(lines, 6);

    drawProgressBar(Layout::cJoyCalProgressY, float(millis()-myCalibrationStartTime-cCenterTotalDuration) / cMinMaxDuration);
  } 
  else if (myResultIsActive == true)
  {
    drawField(rawX, rawY);
    drawSliderX(rawX);
    drawSliderY(rawY);

    const char* axes = (myResult.x == false && myResult.y == false) ? "X and Y axes:"
                     : (myResult.x == false)                        ? "X axis:"
                     :                                                "Y axis:";
    const char* const lines[] = { axes, "not moved.", "Full range", "stored." };
    drawTextLines(lines, 4);
  }
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::drawField(int rawX, int rawY)
{
  const int x0 = Layout::cJoyCalFieldPosX;
  const int y0 = Layout::cJoyCalFieldPosY;
  const int n  = Layout::cJoyCalFieldSize;

  // Range covered so far; y grows upwards on the joystick and downwards on the screen
  canvas().setBrushColor(0, 0, 140);
  canvas().fillRectangle(x0 + axisPos(myMinX, myCenterX, n), y0 + n - axisPos(myMaxY, myCenterY, n),
                         x0 + axisPos(myMaxX, myCenterX, n), y0 + n - axisPos(myMinY, myCenterY, n));

  // Frame and center cross
  canvas().setPenWidth(1);
  canvas().setPenColor(cDefaultCol);
  canvas().drawRectangle(x0, y0, x0 + n, y0 + n);
  canvas().drawLine(x0 + n/2, y0, x0 + n/2, y0 + n);
  canvas().drawLine(x0, y0 + n/2, x0 + n, y0 + n/2);

  // Current position
  canvas().setBrushColor(Color::BrightWhite);
  canvas().fillEllipse(x0 + axisPos(rawX, myCenterX, n), y0 + n - axisPos(rawY, myCenterY, n), cDotSize, cDotSize);
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::drawSliderX(int raw)
{
  const int x0 = Layout::cJoyCalFieldPosX;
  const int n  = Layout::cJoyCalFieldSize;
  const int y0 = Layout::cJoyCalFieldPosY + n + cSliderGap;

  // Bar and dot use the length inside the frame, so the bar keeps its distance at both ends
  const int x1 = x0 + cSliderInset;
  const int m  = n - 2*cSliderInset;

  canvas().setBrushColor(0, 0, 255);
  canvas().fillRectangle(x1 + axisPos(myMinX, myCenterX, m), y0 + cSliderInset,
                         x1 + axisPos(myMaxX, myCenterX, m), y0 + cSliderWidth - cSliderInset);

  canvas().setPenWidth(1);
  canvas().setPenColor(cDefaultCol);
  canvas().drawRectangle(x0, y0, x0 + n, y0 + cSliderWidth);
  canvas().drawLine(x0 + n/2, y0, x0 + n/2, y0 + cSliderWidth);

  canvas().setBrushColor(Color::BrightWhite);
  canvas().fillEllipse(x1 + axisPos(raw, myCenterX, m), y0 + cSliderWidth/2, cSliderDot, cSliderDot);
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::drawSliderY(int raw)
{
  const int n  = Layout::cJoyCalFieldSize;
  const int x0 = Layout::cJoyCalFieldPosX - cSliderGap - cSliderWidth;
  const int y0 = Layout::cJoyCalFieldPosY;

  // Bar and dot use the length inside the frame, so the bar keeps its distance at both ends
  const int y1 = y0 + n - cSliderInset;
  const int m  = n - 2*cSliderInset;

  canvas().setBrushColor(0, 0, 255);
  canvas().fillRectangle(x0 + cSliderInset,                y1 - axisPos(myMaxY, myCenterY, m),
                         x0 + cSliderWidth - cSliderInset, y1 - axisPos(myMinY, myCenterY, m));

  canvas().setPenWidth(1);
  canvas().setPenColor(cDefaultCol);
  canvas().drawRectangle(x0, y0, x0 + cSliderWidth, y0 + n);
  canvas().drawLine(x0, y0 + n/2, x0 + cSliderWidth, y0 + n/2);

  canvas().setBrushColor(Color::BrightWhite);
  canvas().fillEllipse(x0 + cSliderWidth/2, y1 - axisPos(raw, myCenterY, m), cSliderDot, cSliderDot);
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::drawProgressBar(int y, float fraction)
{
  const int w = canvas().getWidth();

  fraction = constrain(fraction, 0.0f, 1.0f);

  canvas().setPenWidth(2);
  canvas().setLineEnds(LineEnds::None);
  canvas().setPenColor(cDefaultCol);
  canvas().drawRectangle(30, y-10, w-30, y+10);
  canvas().setBrushColor(0, 0, 255);
  canvas().fillRectangle(33, y-6, 33 + (w-66)*fraction, y+7);
  canvas().setPenWidth(1);
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::drawTextLines(const char* const lines[], int numLines)
{
  const int lineSpacing = Layout::cJoyCalLargeText ? 22 : 18;

  canvas().setPenColor(cDefaultCol);
  canvas().selectFont(Layout::cJoyCalLargeText ? &fabgl::FONT_std_22 : &fabgl::FONT_std_18);

  // An empty line is a gap of half a line
  int y = Layout::cJoyCalTextPosY;
  for (int i=0; i<numLines; i++)
  {
    if (lines[i][0] == '\0')
    {
      y += lineSpacing/2;
      continue;
    }
    canvas().drawText(Layout::cJoyCalTextPosX, y, lines[i]);
    y += lineSpacing;
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
  myResultIsActive              = false;
  myCalibrationStartTime        = millis();

  setState(eActive);
}

