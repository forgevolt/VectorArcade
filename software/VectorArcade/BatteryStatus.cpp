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

} // namespace

// ----------------------------------------------------------------------------------------
BatteryStatus::BatteryStatus(fabgl::Canvas& canvas, SFE_MAX1704X& batteryMonitor)
: DisplayObject(canvas),
  myFuelGauge(batteryMonitor),
  myLastUpdate(millis()),
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
      
  // Update every 30sec
  if ((millis() - myLastUpdate) > 30000 || myCellPercent < 0)
  { 
    myLastUpdate = millis();
    myCellPercent = myFuelGauge.getSOC();
  }

  myIsCharging = isCharging();
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

  canvas().setPenWidth(1);
  canvas().setBrushColor(col);
  canvas().fillRectangle(cPosX+3, cPosY+3, cPosX+numSeg*cSegWidth+1, cPosY-3+cHeight);

  canvas().setPenColor(Color::Black);
  for (int i=1; i<numSeg; i++)
  {
    canvas().drawLine(cPosX+2+i*cSegWidth, cPosY+3, cPosX+2+i*cSegWidth, cPosY-3+cHeight);
  }

  if (myIsCharging == true)
  {
    canvas().setPenColor(cDefaultCol);
    canvas().selectFont(&fabgl::FONT_std_24);
    canvas().drawText(58, 2, "+");
  }
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

