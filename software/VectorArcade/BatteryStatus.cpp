#include "BatteryStatus.h"
#include <Streaming.h>
#include "Board.h"
#include "Layout.h"

// ---- BatteryStatus ---------------------------------------------------------------------

namespace
{

const int cPosX       = Layout::cBatteryPosX;
const int cPosY       = Layout::cBatteryPosY;
const int cWidth      = Layout::cBatteryWidth;
const int cHeight     = Layout::cBatteryHeight;
const int cSegWidth   = Layout::cBatterySegmentWidth;
const int cTermWidth  = Layout::cBatteryTerminalWidth;
const int cTermHeight = Layout::cBatteryTerminalHalfHeight;

// Thresholds from discharge tests: both consoles' gauges reach 0 % at about 3.3 V, after which
// the cells were drawn far below 3.0 V. The gauge filters the voltage steps of a game, so its
// 0 % is the main "empty" mark; the voltage is the safety net for a gauge that has just started
// and estimated from the present voltage.
const unsigned long cReadInterval    = 5000;   // ms between gauge readings
const float         cLowPercent      = 1.0f;   // below this the last segment blinks
const float         cEmptyPercent    = 0.05f;  // the gauge shows 0.0 %
const float         cEmptyVoltage    = 3.20f;  // V, safety net
const unsigned long cEmptyDuration   = 60000;  // ms the condition must hold
const float         cRechargedPercent = 2.0f;  // leaves the "empty" screen
const unsigned long cBlinkPeriod     = 500;    // ms on, ms off
const unsigned long cChargeStep      = 500;    // ms per segment of the charging animation

// Charge in percent, right of the battery terminal
const int cPercentPosX = cPosX + cWidth + cTermWidth + 9;
const int cPercentPosY = cPosY + cHeight/2 - 7;

} // namespace

// ----------------------------------------------------------------------------------------
BatteryStatus::BatteryStatus(fabgl::Canvas& canvas, SFE_MAX1704X& batteryMonitor)
: DisplayObject(canvas),
  myFuelGauge(batteryMonitor),
  myHasGauge(batteryMonitor.isConnected()),
  myLastReading(0),
  myCellPercent(-1.0),
  myIsCharging(false)
{
#if BOARD_HAS_CHARGE_SENSE()
  pinMode(cUSBStatus, INPUT);
#endif
}

// ----------------------------------------------------------------------------------------
void BatteryStatus::step(unsigned long dt)
{
  if (isActive() == false)
    return;

  myIsCharging = isCharging();
}

// ----------------------------------------------------------------------------------------
void BatteryStatus::monitor()
{
  // Without a gauge every reading would be 0 and the console would stop as "empty"
  if (myHasGauge == false)
    return;

  const unsigned long now = millis();
  if (myCellPercent >= 0 && now - myLastReading < cReadInterval)
    return;
  myLastReading = now;

  myCellPercent = myFuelGauge.getSOC();
  const float voltage = myFuelGauge.getVoltage();

  if (myIsEmpty == true)
  {
    if (myCellPercent >= cRechargedPercent)
    {
      myIsEmpty  = false;
      myLowSince = 0;
    }
    return;
  }

  if (myCellPercent < cEmptyPercent || voltage < cEmptyVoltage)
  {
    if (myLowSince == 0)
      myLowSince = now;
    else if (now - myLowSince >= cEmptyDuration)
      myIsEmpty = true;
  }
  else
  {
    myLowSince = 0;
  }
}

// ----------------------------------------------------------------------------------------
void BatteryStatus::draw()
{
  if (isActive() == false)
    return;

  RGB888 col;
  int numSeg;
 
  if (myCellPercent > 80)
  {
    col = cBattery80_100Col;
    numSeg = 5;
  }
  else if (myCellPercent > 60 && myCellPercent <= 80)
  {
    col = cBattery60_80Col;
    numSeg = 4; 
  }
  else if (myCellPercent > 40 && myCellPercent <= 60)
  {
    col = cBattery40_60Col;
    numSeg = 3;
  }
  else if (myCellPercent > 20 && myCellPercent <= 40)
  {
    col = cBattery20_40Col;
    numSeg = 2;
  }
  else
  {
    col = cBattery00_20Col;
    numSeg = 1;
  }

  canvas().setPenColor(cDefaultCol);
  canvas().setBrushColor(cDefaultCol);

  // ---- Shape of the battery

  canvas().setPenWidth(1);
  canvas().setLineEnds(LineEnds::None);
  canvas().drawRectangle(cPosX, cPosY, cPosX+cWidth, cPosY+cHeight);
  if (Layout::cBatteryDoubleOutline)
    canvas().drawRectangle(cPosX+1, cPosY+1, cPosX-1+cWidth, cPosY-1+cHeight);
  canvas().fillRectangle(cPosX+cWidth+3, cPosY+cHeight/2-cTermHeight, cPosX+cWidth+3+cTermWidth, cPosY+cHeight/2+cTermHeight);

  // ---- Visualize charge percentage

  // While charging, the segments turn green and fill up one by one to full, starting again
  // from the present level (from the fourth segment when it is full already)
  if (myIsCharging == true)
  {
    col = cBattery80_100Col;
    const int first = min(numSeg, 4);
    numSeg = first + (millis() / cChargeStep) % (6 - first);
  }

  // Below cLowPercent the last segment blinks, unless charging
  const bool blinkOff = (myIsCharging == false &&
                         myCellPercent >= 0 && myCellPercent < cLowPercent &&
                         (millis() / cBlinkPeriod) % 2 == 1);

  canvas().setPenWidth(1);
  if (blinkOff == false)
  {
    canvas().setBrushColor(col);
    canvas().fillRectangle(cPosX+3, cPosY+3, cPosX+numSeg*cSegWidth+1, cPosY-3+cHeight);

    canvas().setPenColor(Color::Black);
    for (int i=1; i<numSeg; i++)
    {
      canvas().drawLine(cPosX+2+i*cSegWidth, cPosY+3, cPosX+2+i*cSegWidth, cPosY-3+cHeight);
    }
  }

  // ---- Charge in percent, rounded down: 100 % only when the gauge reads full

  if (myCellPercent >= 0)
  {
    const int percent = constrain((int)myCellPercent, 0, 100);
    canvas().setPenColor(cDefaultCol);
    canvas().selectFont(&fabgl::FONT_std_16);
    canvas().drawTextFmt(cPercentPosX, cPercentPosY, "%d%%", percent);
  }
}

// ----------------------------------------------------------------------------------------
void BatteryStatus::drawEmpty()
{
  canvas().setBrushColor(Color::Black);
  canvas().clear();
  drawHeader(30, "Battery empty");

  // A large battery with one red segment
  const int w  = 90;
  const int h  = 44;
  const int x0 = canvas().getWidth()/2 - 50;
  const int y0 = 90;
  drawFrame(x0, y0, x0 + w, y0 + h);
  canvas().setBrushColor(cDefaultCol);
  canvas().fillRectangle(x0 + w + 3, y0 + h/2 - 8, x0 + w + 9, y0 + h/2 + 8);
  canvas().setBrushColor(cBattery00_20Col);
  canvas().fillRectangle(x0 + 5, y0 + 5, x0 + 14, y0 + h - 5);

  drawCenteredText(155, "Please charge");
  drawCenteredText(185, "or switch off.");
}

//----------------------------------------------------------------------------------------------
bool BatteryStatus::isCharging()
{
#if BOARD_HAS_CHARGE_SENSE()
  if (digitalRead(cUSBStatus) == false)
    return false;

  // ---- Compute charge state from sense pin (rolling average)
  myChargeStatValue -= (myChargeStatValue / 50.0f);
  myChargeStatValue += (analogRead(cChargeStat) / 50.0f);
  myNumChargeReadings++;

  // Wait until stable reading
  if (myNumChargeReadings < 50)
    return false; 

  // Charging: always measured as 0; some outliers may occur!
  else if (myChargeStatValue < 3000) 
    return true;

  else
   return false;
#else
  return false;
#endif
}

