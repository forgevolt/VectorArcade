#pragma once

#include <fabgl.h>
#include "Menu.h"
#include "DisplayObject.h"

// ---- MenuItem --------------------------------------------------------------------------
// Abstract base class for menu entries.

class MenuItem : public DisplayObject
{
  public:
    MenuItem(Menu& menu, const String& name);
    virtual ~MenuItem() {}

    // From DisplayObject (default implementations)
    void step(unsigned long dt) override {}
    void draw() override;

    // Start processing the menu item function
    virtual void start() = 0;

    void setPosY(int16_t y)         { myPosY = y;              }
    void setSelected(bool selected) { myIsSelected = selected; }
    
    SoundEngine& sound() { return myMenu.sound(); }

  protected:
    String myMenuName;  // Menu item description
    int16_t myPosY;     // y position on the screen to render the item
    bool myIsSelected;  // true if item is selected
    Menu& myMenu;       // Reference to the menu, e.g. to access the canvas
};
