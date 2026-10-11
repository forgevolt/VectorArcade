#include "DisplayObject.h"
#include <Streaming.h>
#include "JoystickCalibration.h"
#include "Layout.h"
#include <VectorUtilities.h>
using namespace vectorutilities;
using namespace joystickutilities;


// ---- JoystickCalibration ---------------------------------------------------------------

namespace
{

// Center calibration: a sprung stick rests at a slightly different spot depending on the
// direction it was released from, so the center is averaged over one release per direction
// (up, right, down, left). A is accepted once the stick has been pushed past half-way in that
// direction - the threshold of the menu's direction keys - and the rest position is then
// measured once the console has settled from the button press.
const int cNumReleases = 4;
const int cPushThreshold = DirectionConfig().onThreshold;
const unsigned long cCenterSettleDuration = 200;
const unsigned long cCenterSampleDuration = 1000;

const RGB888 cPushedCol(0, 200, 0);    // Arrow and tick once the stick was pushed
const RGB888 cDimCol(90, 90, 90);      // "press A" while A is not accepted yet

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

// Highest raw value of an axis (12-bit ADC, the joystick's AnalogConfig)
const int cAdcMax = AnalogConfig().adcMax;

// ----------------------------------------------------------------------------------------
// Position of a raw ADC value on an axis that is 'len' pixels long, with the calibrated
// center in the middle: [0, center] fills the first half, [center, cAdcMax] the second,
// so the middle always marks the center and each half shows its own travel.
int axisPos(int raw, int center, int len)
{
  if (raw <= center)
    return mapf(raw, 0, center, 0, len/2);

  return mapf(raw, center, cAdcMax, len/2, len);
}

// ----------------------------------------------------------------------------------------
// True if the stick (joystick values, y up) is pushed past half-way in the direction of
// release 'direction' (0 up, 1 right, 2 down, 3 left)
bool isPushed(int direction, int x, int y)
{
  const int along[cNumReleases] = { y, x, -y, -x };
  return along[direction] > cPushThreshold;
}

} // namespace

// ----------------------------------------------------------------------------------------
JoystickCalibration::JoystickCalibration(Menu& menu)
: MenuItem(menu, "Joystick Calibration"),
  myJoy(menu.input().joy()),
  myRange(std::array<int, 2>{})
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

  // Center calibration: waiting for the stick to be pushed, released and A to be pressed;
  // B cancels without changing the stored calibration
  if (myCenterCalibrationIsActive == true && mySamplingIsActive == false)
  {
    if (isPushed(myRelease, s.x, s.y) == true)
      myPushed = true;

    if (s.b == true)
    {
      myCenterCalibrationIsActive = false;
      setState(eNotActive);
    }
    else if (s.a == true && myPushed == true)
    {
      mySamplingIsActive  = true;
      mySamplingStartTime = millis();
    }
  }

  // Center calibration: measure the rest position of this release, after the settle time
  else if (myCenterCalibrationIsActive == true && millis()-mySamplingStartTime <= cCenterSettleDuration+cCenterSampleDuration)
  {
    if (millis()-mySamplingStartTime > cCenterSettleDuration)
      myCenter.add(myJoy);
  }

  // Release measured -> next release
  else if (myCenterCalibrationIsActive == true && myRelease < cNumReleases-1)
  {
    mySamplingIsActive = false;
    myPushed           = false;
    myRelease++;
    sound().play(sound().signal());
  }

  // All releases measured -> start with min/max calibration
  else if (myCenterCalibrationIsActive == true)
  {
    // Unlikely, but better safe than sorry: without samples there is no center
    if (myCenter.count() == 0)
    {
      myCenterCalibrationIsActive = false;
      setState(eNotActive);
      Serial << __PRETTY_FUNCTION__  << " no center samples " << endl;
      return;
    }

    myRange = RangeTracker<2>(myCenter.center());

    myCenterCalibrationIsActive = false;
    mySamplingIsActive          = false;
    myMinMaxCalibrationIsActive = true;
    myStopCalibration           = false;
    myCalibrationStartTime      = millis();
    sound().play(sound().signal());
  }

  // User has pressed a button while min/max calibration is active -> abort
  else if (myMinMaxCalibrationIsActive == true && myStopCalibration == true && millis()-myCalibrationStartTime <= cMinMaxDuration)
  {
    myStopCalibration = false;
    myCalibrationStartTime += 2*cMinMaxDuration;
  }

  // Min/max calibration: Process current values
  else if (myMinMaxCalibrationIsActive == true && millis()-myCalibrationStartTime <= cMinMaxDuration)
  {
    myRange.add(myJoy);
  }

  // End of the min/max calibration?
  else if (myMinMaxCalibrationIsActive == true && millis()-myCalibrationStartTime > cMinMaxDuration)
  {
    myMinMaxCalibrationIsActive = false;
    myResult = myJoy.setCalibration(myRange.calibration());

    // Stored as applied: a rejected axis stores the full range it fell back to
    saveCalibration(myJoy, cJoystickNameSpace);

    if (myResult.allAccepted())
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

  drawCenteredText(30, myCenterCalibrationIsActive ? "Center Calibration" : "Min/Max Calibration");
  canvas().setPenColor(cDefaultCol);
  canvas().drawLine(0, 60, canvas().getWidth(), 60);

  // One reading per frame for the dots
  int rawX = myJoy.readRaw(0);
  int rawY = myJoy.readRaw(1);

  if (myCenterCalibrationIsActive == true)
  {
    float released = myRelease;

    if (mySamplingIsActive == false)
    {
      drawCenteredText(66, "Push the stick,");
      drawArrow(myRelease, myPushed);

      canvas().setPenColor(myPushed ? cDefaultCol : cDimCol);
      drawCenteredText(164, "let it go, press A.");
      canvas().setPenColor(cDefaultCol);
    }
    else
    {
      drawCenteredText(66, "Don't move the joystick!");
      released += float(millis()-mySamplingStartTime) / (cCenterSettleDuration+cCenterSampleDuration);
    }

    drawProgressBar(203, released / cNumReleases);

    // Counter bottom left, "B cancels." bottom right
    canvas().selectFont(&fabgl::FONT_std_16);
    canvas().setPenColor(cDefaultCol);
    canvas().drawText(30, 220, (String(myRelease+1) + " / " + String(cNumReleases)).c_str());
    if (mySamplingIsActive == false)
      canvas().drawText(canvas().getWidth() - 30 - canvas().textExtent("B cancels."), 220, "B cancels.");
  }
  else if (myMinMaxCalibrationIsActive == true)
  {
    drawField(rawX, rawY);
    drawSliderX(rawX);
    drawSliderY(rawY);

    const char* const lines[] = { "Move the", "joystick to", "all edges.", "", "A button", "exits." };
    drawTextLines(lines, 6);

    drawProgressBar(Layout::cJoyCalProgressY, float(millis()-myCalibrationStartTime) / cMinMaxDuration);
  } 
  else if (myResultIsActive == true)
  {
    drawField(rawX, rawY);
    drawSliderX(rawX);
    drawSliderY(rawY);

    const char* axes = (myResult.accepted[0] == false && myResult.accepted[1] == false) ? "X and Y axes:"
                     : (myResult.accepted[0] == false)                                  ? "X axis:"
                     :                                                                    "Y axis:";
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
  const RangeTracker<2>& r = myRange;
  canvas().setBrushColor(0, 0, 140);
  canvas().fillRectangle(x0 + axisPos(r.min(0), r.center(0), n), y0 + n - axisPos(r.max(1), r.center(1), n),
                         x0 + axisPos(r.max(0), r.center(0), n), y0 + n - axisPos(r.min(1), r.center(1), n));

  // Frame and center cross
  canvas().setPenWidth(1);
  canvas().setPenColor(cDefaultCol);
  canvas().drawRectangle(x0, y0, x0 + n, y0 + n);
  canvas().drawLine(x0 + n/2, y0, x0 + n/2, y0 + n);
  canvas().drawLine(x0, y0 + n/2, x0 + n, y0 + n/2);

  // Current position
  canvas().setBrushColor(Color::BrightWhite);
  canvas().fillEllipse(x0 + axisPos(rawX, r.center(0), n), y0 + n - axisPos(rawY, r.center(1), n), cDotSize, cDotSize);
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
  canvas().fillRectangle(x1 + axisPos(myRange.min(0), myRange.center(0), m), y0 + cSliderInset,
                         x1 + axisPos(myRange.max(0), myRange.center(0), m), y0 + cSliderWidth - cSliderInset);

  canvas().setPenWidth(1);
  canvas().setPenColor(cDefaultCol);
  canvas().drawRectangle(x0, y0, x0 + n, y0 + cSliderWidth);
  canvas().drawLine(x0 + n/2, y0, x0 + n/2, y0 + cSliderWidth);

  canvas().setBrushColor(Color::BrightWhite);
  canvas().fillEllipse(x1 + axisPos(raw, myRange.center(0), m), y0 + cSliderWidth/2, cSliderDot, cSliderDot);
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
  canvas().fillRectangle(x0 + cSliderInset,                y1 - axisPos(myRange.max(1), myRange.center(1), m),
                         x0 + cSliderWidth - cSliderInset, y1 - axisPos(myRange.min(1), myRange.center(1), m));

  canvas().setPenWidth(1);
  canvas().setPenColor(cDefaultCol);
  canvas().drawRectangle(x0, y0, x0 + cSliderWidth, y0 + n);
  canvas().drawLine(x0, y0 + n/2, x0 + cSliderWidth, y0 + n/2);

  canvas().setBrushColor(Color::BrightWhite);
  canvas().fillEllipse(x0 + cSliderWidth/2, y1 - axisPos(raw, myRange.center(1), m), cSliderDot, cSliderDot);
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
void JoystickCalibration::drawArrow(int direction, bool pushed)
{
  const int cx = canvas().getWidth()/2;
  const int cy = 126;

  // Pointing up, relative to the center: shaft and head, 64 pixels long
  const int shape[7][2] = { {-8, 32}, {-8, -4}, {-20, -4}, {0, -32}, {20, -4}, {8, -4}, {8, 32} };

  // Turned by 90 degrees clockwise per direction
  Point p[7];
  for (int i=0; i<7; i++)
  {
    int x = shape[i][0];
    int y = shape[i][1];
    for (int r=0; r<direction; r++)
    {
      const int t = x;
      x = -y;
      y = t;
    }
    p[i] = Point(cx + x, cy + y);
  }

  if (pushed == true)
  {
    canvas().setBrushColor(cPushedCol);
    canvas().fillPath(p, 7);

    // Tick right of the arrow
    canvas().setPenColor(cPushedCol);
    canvas().setPenWidth(2);
    canvas().drawLine(cx + 48, cy,     cx + 56, cy + 9);
    canvas().drawLine(cx + 56, cy + 9, cx + 72, cy - 12);
  }
  else
  {
    canvas().setPenColor(cDefaultCol);
    canvas().drawPath(p, 7);
  }

  canvas().setPenWidth(1);
  canvas().setPenColor(cDefaultCol);
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::start()
{
  myCenter.reset();
  
  myCenterCalibrationIsActive   = true;
  myRelease                     = 0;
  myPushed                      = false;
  mySamplingIsActive            = false;
  myMinMaxCalibrationIsActive   = false;
  myStopCalibration             = false;
  myResultIsActive              = false;

  setState(eActive);
}

