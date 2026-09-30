#include <Streaming.h>

#include "Menu.h"
#include "MenuItem.h"
#include "PinMap.h"

// ---- Menu ------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
Menu::Menu(fabgl::Canvas& canvas)
: myCanvas(canvas),
  mySound(cI2S_LRC, cI2S_BCLK, cI2S_DOUT),
  myActiveItem(0)
{}

// ----------------------------------------------------------------------------------------
Menu::~Menu()
{
  for (auto item : myMenuItems)
  {
    delete item;
  }

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

  int16_t y = 60;
  for (auto item : myMenuItems)
  {
    item->setPosY(y);
    item->setSelected(false);
    item->setState(DisplayObject::eNotActive);
    y += 45;
  }

  setActiveItem(0);

  return true;
}

// ----------------------------------------------------------------------------------------
void Menu::step(unsigned long dt)
{
  // No registered MenuItems?
  if (myMenuItems.empty() == true)
    return;

  // Is the processing of MenuItem in progress?
  if (myMenuItems[myActiveItem]->isActive() == true)
  {
    myMenuItems[myActiveItem]->step(dt);
  }

  // No active item
  else
  {
    // ---- Process input events (up, down, select)
    InputController::State s = myInput.getState();

    if (s.start == true || s.joy == true || s.a == true)
    {
      // Start processing the MenuItem
      mySound.play(mySound.click());
      myMenuItems[myActiveItem]->start();
      return;
    }

    if (s.down == true)
      setActiveItem(myActiveItem+1);  

    if (s.up == true)
      setActiveItem(myActiveItem-1);  

    /*
    // Each MenuItem might have its own animation (e.g. space ship icon shoots a laser)
    for (auto item : myMenuItems)
    {
      item->step(dt); 
    } 
    */

    for (auto object : myObjects)
    {
      object->step(dt); 
    } 

  }
}

// ----------------------------------------------------------------------------------------
void Menu::draw()
{
  // No registered MenuItems?
  if (myMenuItems.empty() == true)
    return;

  // Processing of MenuItem is in progress
  else if (myMenuItems[myActiveItem]->isActive() == true)
    myMenuItems[myActiveItem]->draw();

  else
  {
    // Clear screen
    myCanvas.setBrushColor(Color::Black);
    myCanvas.clear();
    myCanvas.setGlyphOptions(GlyphOptions().FillBackground(false));

    for (auto item : myMenuItems)
    {
      item->draw();
    }

    for (auto object : myObjects)
    {
      object->draw(); 
    }

    // LCD outline to check opening of the housing
    // myCanvas.setPenColor(cDefaultCol);
    // myCanvas.drawRectangle(0, 0, myCanvas.getWidth()-1, myCanvas.getHeight()-1);
  }
}

// ----------------------------------------------------------------------------------------
void Menu::addMenu(MenuItem* adoptMenuItem)
{
  if (adoptMenuItem == nullptr)
  {
    Serial << __PRETTY_FUNCTION__  << " -> adoptMenuItem == nullptr" << endl;
    return;
  }

  myMenuItems.push_back(adoptMenuItem);
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

// ----------------------------------------------------------------------------------------
void Menu::setActiveItem(int num)
{
  if (myMenuItems.empty() == true)
    return;

  // Processing of MenuItem is already in progress -> do not change active MenuItem
  if (myMenuItems[myActiveItem]->isActive() == true)
    return;

  if (num < 0)
    num = myMenuItems.size()-1;
  else if (num >= (int)myMenuItems.size())
    num = 0;

  myMenuItems[myActiveItem]->setSelected(false);
  myMenuItems[num]->setSelected(true);

  myActiveItem = num;
}
