#include "MenuPage.h"
#include <Streaming.h>

// ---- MenuPage --------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
MenuPage::MenuPage(Menu& menu, const String& name)
: MenuItem(menu, name),
  myActiveItem(0)
{}

// ----------------------------------------------------------------------------------------
MenuPage::~MenuPage()
{
  for (auto item : myMenuItems)
  {
    delete item;
  }
}

// ----------------------------------------------------------------------------------------
void MenuPage::step(unsigned long dt)
{
  // No registered MenuItems?
  if (myMenuItems.empty() == true)
    return;

  // Is processing of MenuItem in progress?
  if (myMenuItems[myActiveItem]->isActive() == true)
  {
    myMenuItems[myActiveItem]->step(dt);
  }

  // No active item
  else
  {
    // ---- Process input events (up, down, select)
    InputController::State s = myMenu.input().getState();

    if (s.start == true || s.joy == true || s.a)
    {
      // Start processing the MenuItem
      sound().play(sound().click());
      myMenuItems[myActiveItem]->start();
      return;
    }

    if (s.sel == true)
    {
      sound().play(sound().click());
      setState(eNotActive);
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

    myMenu.stepObjects(dt);
  }
}

// ----------------------------------------------------------------------------------------
void MenuPage::draw()
{
  if (isActive() == false)
  {
    MenuItem::draw();
    canvas().drawText(canvas().getWidth()-45, myPosY-10, ">");
  }
  else
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
      canvas().setBrushColor(Color::Black);
      canvas().clear();
      canvas().setGlyphOptions(GlyphOptions().FillBackground(false));

      for (auto item : myMenuItems)
      {
        item->draw();
      }

      myMenu.drawObjects();
    }
  }
}

// ----------------------------------------------------------------------------------------
void MenuPage::add(MenuItem* adoptMenuItem)
{
  if (adoptMenuItem == nullptr)
  {
    Serial << __PRETTY_FUNCTION__  << " -> adoptMenuItem == nullptr" << endl;
    return;
  }

  myMenuItems.push_back(adoptMenuItem);
}

// ----------------------------------------------------------------------------------------
void MenuPage::start()
{
  int16_t y = 60;
  for (auto item : myMenuItems)
  {
    item->setPosY(y);
    item->setSelected(false);
    item->setState(DisplayObject::eNotActive);
    y += 45;
  }

  setActiveItem(0);
  setState(eActive);
}

// ----------------------------------------------------------------------------------------
void MenuPage::setActiveItem(int num)
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