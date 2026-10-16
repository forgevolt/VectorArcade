#include "DisplayObject.h"
#include <Streaming.h>
#include "JoystickCalibration.h"
using namespace joystickutilities;


// ---- JoystickCalibration ---------------------------------------------------------------

namespace
{

// Center calibration: a sprung stick rests at a slightly different spot depending on the
// direction it was released from, so the center is the average of the rest positions after one
// release per direction. A release is detected without a button: pushed past the on-threshold
// of the direction keys, back under their off-threshold, every axis still for cStillDuration,
// then measured for cMeasureDuration. A new push in the step's direction starts it over.
// A released stick is still after about 0.3 - 0.5 s; at rest the noise stays within a few counts.
const unsigned long cStillDuration   = 200;  // every axis within cStillCounts for this long (ms) ...
const int           cStillCounts     = 15;   // ... counts as holding still
const unsigned long cMeasureDuration = 300;  // then the rest position is averaged for this long (ms)
const unsigned long cDoneDuration    = 400;  // the check mark stays this long (ms)

// The release steps: axis (0 x, 1 y), direction of the push and the prompt
struct ReleaseStep
{
  size_t axis;
  int sign;
  const char* prompt;
};

const ReleaseStep cReleaseSteps[] =
{
  { 1, +1, "Push up and let go" },   { 1, -1, "Push down and let go" },
  { 0, -1, "Push left and let go" }, { 0, +1, "Push right and let go" }
};

const int cNumReleases = sizeof(cReleaseSteps) / sizeof(cReleaseSteps[0]);

// Layout of the center calibration (pixels)
const int cStickY      = 146;                         // center of the joystick drawing
const int cStickR      = 36;                          // radius of its circle
const int cStickDotR   = 6;                           // the dot that follows the stick ...
const int cStickTravel = cStickR - cStickDotR - 2;    // ... this far from the center at full deflection
const int cStatusY     = 200;                         // the state in words
const int cProgressY   = 229;                         // one dot per release

// Total of 10 seconds to move both axes to their min/max positions
const unsigned long cMinMaxDuration = 10000;

// Time the edges not reached are reported on screen, unless a button is pressed
const unsigned long cResultDuration = 5000;

// Layout of the min/max calibration (pixels): a circle like the one of the center calibration,
// with the range covered so far inside and one marker per edge outside
const int cRangeR      = 40;                         // radius of the circle
const int cRangeDotR   = 6;                          // the dot that follows the stick ...
const int cRangeTravel = cRangeR - cRangeDotR - 2;   // ... this far from the center at full deflection
const int cRangePoints = 8;                          // corners of the range shape per quarter
const int cTimeBarY    = 229;                        // center line of the bar showing the time left
const int cTimeBarHalf = 40;                         // half its length

// An edge counts as reached once the stick has covered this share of the way from the center
// to the end of the ADC range. The range is applied and stored only if every edge is reached.
const float cEdgeReached = 0.75f;

// The edges in the order of the range shape's quarters, counterclockwise from the right
struct Edge
{
  size_t axis;
  int sign;
};

const Edge cEdges[4] = { { 0, +1 }, { 1, +1 }, { 0, -1 }, { 1, -1 } };   // right, up, left, down

// Highest raw value of an axis (12-bit ADC, the joystick's AnalogConfig)
const int cAdcMax = AnalogConfig().adcMax;

// ----------------------------------------------------------------------------------------
// Share of the way from the calibrated center to the end of the ADC range in the direction of
// 'raw': 0 at the center, +1 at cAdcMax, -1 at 0.
float travel(int raw, int center)
{
  if (raw >= center)
    return (center < cAdcMax) ? float(raw - center) / (cAdcMax - center) : 0.0f;

  return (center > 0) ? -float(center - raw) / center : 0.0f;
}

// ----------------------------------------------------------------------------------------
// Share of the way to an edge covered so far
float reach(const RangeTracker<2>& r, const Edge& e)
{
  return (e.sign > 0) ? travel(r.max(e.axis), r.center(e.axis)) : -travel(r.min(e.axis), r.center(e.axis));
}

// ----------------------------------------------------------------------------------------
// Number of edges reached so far
int edgesReached(const RangeTracker<2>& r)
{
  int n = 0;
  for (const Edge& e : cEdges)
    if (reach(r, e) >= cEdgeReached)
      n++;

  return n;
}

// ----------------------------------------------------------------------------------------
// A point of a shape drawn pointing up relative to (cx, cy), turned to point in the direction
// of 'axis' (0 x, 1 y) and 'sign'
Point turned(int x, int y, size_t axis, int sign, int cx, int cy)
{
  if (axis == 0)          // right (sign +1) or left (sign -1)
    return Point(cx - y*sign, cy + x*sign);

  if (sign < 0)           // down
    return Point(cx - x, cy - y);

  return Point(cx + x, cy + y);
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

  // Sel leaves at any time, as in the games. Before the end of the min/max phase the stored
  // calibration is unchanged.
  if (s.sel == true)
  {
    sound().play(sound().click());
    myCenterCalibrationIsActive = false;
    myMinMaxCalibrationIsActive = false;
    myResultIsActive            = false;
    setState(eNotActive);
    return;
  }

  if (s.a || s.b || s.start)
    myStopCalibration = true;

  // Reporting the edges not reached? -> leave after a while or on a button
  if (myResultIsActive == true)
  {
    if (myStopCalibration == true || millis()-myResultStartTime > cResultDuration)
    {
      myResultIsActive = false;
      setState(eNotActive);
    }
    return;
  }

  // Center calibration: the releases, detected without a button
  if (myCenterCalibrationIsActive == true)
  {
    stepRelease();
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

    Serial << "Joystick: range X " << myRange.min(0) << " " << myRange.center(0) << " " << myRange.max(0)
           << ", Y " << myRange.min(1) << " " << myRange.center(1) << " " << myRange.max(1) << endl;

    // Applied and stored only with every edge reached, otherwise the current calibration stays.
    // An axis that reached both edges is far beyond the minimum travel setCalibration() checks.
    if (edgesReached(myRange) == 4)
    {
      myJoy.setCalibration(myRange.calibration());
      saveCalibration(myJoy, cJoystickNameSpace);
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

  drawHeader(30, myCenterCalibrationIsActive ? "Center Calibration" : "Min/Max Calibration");

  if (myCenterCalibrationIsActive == true)
  {
    drawCenteredText(66, cReleaseSteps[myRelease].prompt);
    drawStick();
  }
  else if (myMinMaxCalibrationIsActive == true)
  {
    drawCenteredText(66, "Push to all four edges");
    drawRange(false);
    drawTimeLeft(1.0f - float(millis()-myCalibrationStartTime) / cMinMaxDuration);
  }
  else if (myResultIsActive == true)
  {
    drawCenteredText(66, "Not all edges reached");
    drawRange(true);
  }
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::stepRelease()
{
  const ReleaseStep& current = cReleaseSteps[myRelease];
  const DirectionConfig directions;

  // A push in the step's direction starts the release over, also while settling or measuring
  if (myReleaseState != ERelease::eDone && myJoy.get(current.axis) * current.sign > directions.onThreshold)
    myReleaseState = ERelease::eLetGo;

  switch (myReleaseState)
  {
    case ERelease::ePush:
      break;

    case ERelease::eLetGo:
      if (abs(myJoy.getX()) < directions.offThreshold && abs(myJoy.getY()) < directions.offThreshold)
      {
        myReleaseState     = ERelease::eSettle;
        myReleaseStateTime = millis();
        for (size_t i=0; i<2; i++)   // the still window starts here
          myStillMin[i] = myStillMax[i] = myJoy.readRaw(i);
      }
      break;

    case ERelease::eSettle:
    case ERelease::eMeasure:
      if (isStill() == false)
      {
        myReleaseState     = ERelease::eSettle;
        myReleaseStateTime = millis();
        break;
      }

      if (myReleaseState == ERelease::eSettle && millis()-myReleaseStateTime >= cStillDuration)
      {
        myReleaseState     = ERelease::eMeasure;
        myReleaseStateTime = millis();
        myCenter.reset();
      }

      if (myReleaseState == ERelease::eMeasure)
      {
        myCenter.add(myJoy);
        if (millis()-myReleaseStateTime >= cMeasureDuration)
        {
          const std::array<int, 2> rest = myCenter.center();
          for (size_t i=0; i<2; i++)
            myRestSum[i] += rest[i];

          myReleaseState     = ERelease::eDone;
          myReleaseStateTime = millis();
          sound().play(sound().signal());

          Serial << "Joystick: release " << (myRelease + 1) << " of " << cNumReleases
                 << " at rest " << rest[0] << " " << rest[1] << endl;
        }
      }
      break;

    case ERelease::eDone:
      if (millis()-myReleaseStateTime >= cDoneDuration)
      {
        if (++myRelease < cNumReleases)
          myReleaseState = ERelease::ePush;
        else
          startMinMax();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------------
bool JoystickCalibration::isStill()
{
  std::array<int, 2> raw;
  bool still = true;

  for (size_t i=0; i<2; i++)
  {
    raw[i] = myJoy.readRaw(i);
    myStillMin[i] = min(myStillMin[i], raw[i]);
    myStillMax[i] = max(myStillMax[i], raw[i]);
    if (myStillMax[i] - myStillMin[i] > cStillCounts)
      still = false;
  }

  if (still == false)
    myStillMin = myStillMax = raw;

  return still;
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::startMinMax()
{
  // Rounded average of the rest positions
  std::array<int, 2> center;
  for (size_t i=0; i<2; i++)
    center[i] = int((myRestSum[i] + cNumReleases/2) / cNumReleases);

  myRange = RangeTracker<2>(center);

  myCenterCalibrationIsActive = false;
  myMinMaxCalibrationIsActive = true;
  myStopCalibration           = false;   // a press during the releases does not end this phase
  myCalibrationStartTime      = millis();
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::drawStick()
{
  const ReleaseStep& current = cReleaseSteps[myRelease];
  const int cx = canvas().getWidth()/2;
  const int cy = cStickY;
  const bool done = (myReleaseState == ERelease::eDone);

  const char* word  = "push";
  RGB888      color = Color::BrightWhite;
  switch (myReleaseState)
  {
    case ERelease::ePush:    word = "push";       color = Color::BrightWhite; break;
    case ERelease::eLetGo:   word = "let go";     color = cLetGoCol;          break;
    case ERelease::eSettle:
    case ERelease::eMeasure: word = "hold still"; color = cHoldStillCol;      break;
    case ERelease::eDone:    word = "done";       color = cConfirmCol;        break;
  }

  // Circle, with its center while the stick is still to be released
  canvas().setPenWidth(1);
  canvas().setPenColor(cDefaultCol);
  canvas().drawEllipse(cx, cy, 2*cStickR, 2*cStickR);
  if (done == false)
  {
    canvas().setBrushColor(cDefaultCol);
    canvas().fillEllipse(cx, cy, 5, 5);
  }

  if (done == true)
  {
    // Check mark; round line ends close the gap where the two strokes meet
    canvas().setPenColor(color);
    canvas().setPenWidth(5);
    canvas().setLineEnds(LineEnds::Circle);
    canvas().drawLine(cx - 16, cy + 1,  cx - 6,  cy + 11);
    canvas().drawLine(cx - 6,  cy + 11, cx + 16, cy - 12);
    canvas().setLineEnds(LineEnds::None);
    canvas().setPenWidth(1);
  }
  else
  {
    // Arrow in the step's direction: drawn pointing up, relative to the center, then turned
    const int shape[7][2] = { {-3, -10}, {-3, -38}, {-9, -38}, {0, -52}, {9, -38}, {3, -38}, {3, -10} };

    Point p[7];
    for (int i=0; i<7; i++)
      p[i] = turned(shape[i][0], shape[i][1], current.axis, current.sign, cx, cy);

    canvas().setBrushColor(color);
    canvas().fillPath(p, 7);

    // Where the stick is, with a black ring so that it stands out on the arrow
    const int dx = cx + myJoy.getX() * cStickTravel / cMAX;
    const int dy = cy - myJoy.getY() * cStickTravel / cMAX;
    canvas().setBrushColor(Color::Black);
    canvas().fillEllipse(dx, dy, 2*cStickDotR + 4, 2*cStickDotR + 4);
    canvas().setBrushColor(Color::BrightWhite);
    canvas().fillEllipse(dx, dy, 2*cStickDotR, 2*cStickDotR);
  }

  // The state in words
  canvas().selectFont(&fabgl::FONT_std_16);
  canvas().setPenColor(color);
  drawCenteredText(cStatusY, word);
  canvas().setPenColor(cDefaultCol);

  // One dot per release, filled once it is measured
  for (int i=0; i<cNumReleases; i++)
  {
    const int x = cx + (2*i - (cNumReleases-1)) * 11;
    if (i < myRelease || (i == myRelease && done == true))
    {
      canvas().setBrushColor(cConfirmCol);
      canvas().fillEllipse(x, cProgressY, 11, 11);
    }
    else
    {
      canvas().drawEllipse(x, cProgressY, 11, 11);
    }
  }
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::drawRange(bool result)
{
  const int cx = canvas().getWidth()/2;
  const int cy = cStickY;
  const RangeTracker<2>& r = myRange;

  // Share of the way to each edge covered so far
  float share[4];
  for (int i=0; i<4; i++)
    share[i] = reach(r, cEdges[i]);
  const int reached = edgesReached(r);

  // Range covered so far: a quarter ellipse between each two neighbouring edges
  Point shape[4*cRangePoints];
  for (int q=0; q<4; q++)
  {
    const float rx = share[(q == 1 || q == 2) ? 2 : 0] * (cRangeR - 1);
    const float ry = share[(q < 2) ? 1 : 3] * (cRangeR - 1);
    for (int k=0; k<cRangePoints; k++)
    {
      const float a = (q*cRangePoints + k) * (HALF_PI / cRangePoints);
      shape[q*cRangePoints + k] = Point(cx + lroundf(cosf(a) * rx), cy - lroundf(sinf(a) * ry));
    }
  }

  canvas().setPenWidth(1);
  canvas().setBrushColor(cRangeCol);
  canvas().fillPath(shape, 4*cRangePoints);
  canvas().setPenColor(cBarCol);
  canvas().drawPath(shape, 4*cRangePoints);

  // Circle and its center, as in the center calibration
  canvas().setPenColor(cDefaultCol);
  canvas().drawEllipse(cx, cy, 2*cRangeR, 2*cRangeR);
  canvas().setBrushColor(cDefaultCol);
  canvas().fillEllipse(cx, cy, 5, 5);

  // One marker per edge, pointing outwards: filled once reached. On the result the edges not
  // reached are filled too, in their own color.
  const int marker[3][2] = { {-6, -(cRangeR + 3)}, {0, -(cRangeR + 11)}, {6, -(cRangeR + 3)} };
  for (int i=0; i<4; i++)
  {
    Point p[3];
    for (int j=0; j<3; j++)
      p[j] = turned(marker[j][0], marker[j][1], cEdges[i].axis, cEdges[i].sign, cx, cy);

    const bool ok = (share[i] >= cEdgeReached);
    if (ok == true || result == true)
    {
      const RGB888 color = (ok == true) ? cConfirmCol : cMissedCol;
      canvas().setBrushColor(color);
      canvas().setPenColor(color);
      canvas().fillPath(p, 3);
    }
    else
    {
      canvas().setPenColor(cDefaultCol);
    }
    canvas().drawPath(p, 3);
  }

  // Where the stick is, kept inside the circle on the diagonals, with a black ring so that it
  // stands out on the range
  if (result == false)
  {
    float dx = travel(myJoy.readRaw(0), r.center(0));
    float dy = travel(myJoy.readRaw(1), r.center(1));
    const float len = sqrtf(dx*dx + dy*dy);
    if (len > 1.0f)
    {
      dx /= len;
      dy /= len;
    }
    const int px = cx + lroundf(dx * cRangeTravel);
    const int py = cy - lroundf(dy * cRangeTravel);
    canvas().setBrushColor(Color::Black);
    canvas().fillEllipse(px, py, 2*cRangeDotR + 4, 2*cRangeDotR + 4);
    canvas().setBrushColor(Color::BrightWhite);
    canvas().fillEllipse(px, py, 2*cRangeDotR, 2*cRangeDotR);
  }

  // The edges reached in words
  char count[16];
  const char* word  = count;
  RGB888      color = Color::BrightWhite;
  if (result == true)
  {
    word  = "calibration not stored";
    color = cMissedCol;
  }
  else if (reached == 4)
  {
    word  = "all edges - A finishes";
    color = cConfirmCol;
  }
  else
  {
    snprintf(count, sizeof(count), "%d of 4 edges", reached);
  }

  canvas().selectFont(&fabgl::FONT_std_16);
  canvas().setPenColor(color);
  drawCenteredText(cStatusY, word);
  canvas().setPenColor(cDefaultCol);
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::drawTimeLeft(float fraction)
{
  const int x0 = canvas().getWidth()/2 - cTimeBarHalf;
  const int w  = lroundf((2*cTimeBarHalf - 3) * constrain(fraction, 0.0f, 1.0f));

  canvas().setPenWidth(1);
  canvas().setPenColor(cDefaultCol);
  canvas().drawRectangle(x0, cTimeBarY - 4, x0 + 2*cTimeBarHalf, cTimeBarY + 4);
  if (w > 0)
  {
    canvas().setBrushColor(cBarCol);
    canvas().fillRectangle(x0 + 2, cTimeBarY - 2, x0 + 1 + w, cTimeBarY + 2);
  }
}

// ----------------------------------------------------------------------------------------
void JoystickCalibration::start()
{
  myCenterCalibrationIsActive   = true;
  myRelease                     = 0;
  myReleaseState                = ERelease::ePush;
  myRestSum.fill(0);
  myMinMaxCalibrationIsActive   = false;
  myStopCalibration             = false;
  myResultIsActive              = false;

  setState(eActive);
}

