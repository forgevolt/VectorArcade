#include <Streaming.h>

#include "Menu.h"
#include "MenuItem.h"
#include "BatteryStatus.h"
#include "PinMap.h"

// ---- Menu ------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
Menu::Menu(fabgl::Canvas& canvas)
: myCanvas(canvas),
  mySound(cI2S_LRC, cI2S_BCLK, cI2S_DOUT),
  myItems(mySound)
{}

// ----------------------------------------------------------------------------------------
Menu::~Menu()
{
  for (auto object : myObjects)
  {
    delete object;
  }
}

// ----------------------------------------------------------------------------------------
bool Menu::begin()
{
  // The sound task runs on core 0. On core 1 it shares the CPU and the library's mutex with
  // loop() at the same priority, which cost about 3 ms per frame in gameplay; the frame
  // budget at 25 FPS leaves only ~7 ms next to the display transfer.
  if (mySound.begin(tskIDLE_PRIORITY + 1, 0) == false)
  {
    Serial << "SoundEngine: I2S output could not be started" << endl;
  }
  myInput.begin();

  myItems.reset();

  return true;
}

// ----------------------------------------------------------------------------------------
void Menu::step(unsigned long dt)
{
  // Empty battery: stop whatever runs and wait for the charger. Button presses are read and
  // dropped, so none of them starts an item once the battery is charged again.
  if (myBattery != nullptr)
  {
    myBattery->monitor();
    if (myBattery->isEmpty() == true)
    {
      if (myItems.isItemRunning() == true)
      {
        mySound.stopAll();
        myItems.stopRunningItem();
      }
      myInput.getState();
      return;
    }
  }

  // Is the processing of a MenuItem in progress?
  if (myItems.isItemRunning() == true)
  {
    myItems.stepRunningItem(dt);
    return;
  }

  // ---- Process input events (up, down, select)
  if (myItems.navigate(myInput.getState()) == true)
    return;

  stepObjects(dt);
}

// ----------------------------------------------------------------------------------------
void Menu::draw()
{
  if (myBattery != nullptr && myBattery->isEmpty() == true)
  {
    myBattery->drawEmpty();
    return;
  }

  // Processing of a MenuItem is in progress
  if (myItems.isItemRunning() == true)
  {
    myItems.drawRunningItem();
    return;
  }

  myItems.drawList(myCanvas);
  drawObjects();

  // LCD outline to check opening of the housing
  // myCanvas.setPenColor(cDefaultCol);
  // myCanvas.drawRectangle(0, 0, myCanvas.getWidth()-1, myCanvas.getHeight()-1);
}

// ----------------------------------------------------------------------------------------
void Menu::addMenu(MenuItem* adoptMenuItem)
{
  myItems.add(adoptMenuItem);
}

// ----------------------------------------------------------------------------------------
void Menu::addBatteryStatus(BatteryStatus* adoptBattery)
{
  addObject(adoptBattery);
  myBattery = adoptBattery;
}

// ----------------------------------------------------------------------------------------
void Menu::addObject(DisplayObject* adoptObject)
{
  if (adoptObject == nullptr)
  {
    Serial << __PRETTY_FUNCTION__  << " -> adoptObject == nullptr" << endl;
    return;
  }

  myObjects.push_back(adoptObject);
}


// ----------------------------------------------------------------------------------------
void Menu::stepObjects(unsigned long dt)
{
  for (auto object : myObjects)
  {
    object->step(dt); 
  } 
}

// ----------------------------------------------------------------------------------------
void Menu::drawObjects()
{
  for (auto object : myObjects)
  {
    object->draw(); 
  } 
}
