#include "MenuPage.h"
#include <Streaming.h>

// ---- MenuPage --------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
MenuPage::MenuPage(Menu& menu, const String& name)
: MenuItem(menu, name),
  myItems(menu.sound())
{}

// ----------------------------------------------------------------------------------------
void MenuPage::step(unsigned long dt)
{
  // Is the processing of a MenuItem in progress?
  if (myItems.isItemRunning() == true)
  {
    myItems.stepRunningItem(dt);
    return;
  }

  // ---- Process input events (up, down, select; Sel leaves the page)
  InputController::State s = myMenu.input().getState();

  if (s.sel == true)
  {
    sound().play(sound().click());
    setState(eNotActive);
    return;
  }

  if (myItems.navigate(s) == true)
    return;

  myMenu.stepObjects(dt);
}

// ----------------------------------------------------------------------------------------
void MenuPage::draw()
{
  if (isActive() == false)
  {
    MenuItem::draw();
    canvas().drawText(canvas().getWidth()-45, myPosY-10, ">");
  }
  else if (myItems.isItemRunning() == true)
  {
    myItems.drawRunningItem();
  }
  else
  {
    myItems.drawList(canvas());
    myMenu.drawObjects();
  }
}

// ----------------------------------------------------------------------------------------
void MenuPage::add(MenuItem* adoptMenuItem)
{
  myItems.add(adoptMenuItem);
}

// ----------------------------------------------------------------------------------------
void MenuPage::start()
{
  myItems.reset();
  setState(eActive);
}
