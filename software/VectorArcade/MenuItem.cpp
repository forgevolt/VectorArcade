#include "MenuItem.h"

// ---- MenuItem --------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
MenuItem::MenuItem(Menu& menu, const String& name)
: DisplayObject(menu.canvas()), 
  myMenuName(name),
  myPosY(0),
  myIsSelected(false),
  myMenu(menu)
{}

// ----------------------------------------------------------------------------------------
void MenuItem::draw()
{
  drawButton(20, myPosY, canvas().getWidth()-40, myMenuName, myIsSelected);
}
