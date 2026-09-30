#pragma once

#include <vector>
#include <fabgl.h>

#include "DisplayObject.h"
#include "InputController.h"
#include <SoundEngine.h>


// ---- Menu ------------------------------------------------------------------------------
// Base class for the menu system. A menu consists of a number of MenuItems. A MenuItem may 
// represent a device configuration functionality such as the joystick calibration or an 
// entire game that is started from the menu.

class MenuItem;

class Menu
{
  public:
    Menu(fabgl::Canvas& canvas);
    ~Menu();

    // Begin the processing, returns true if operation is successful.
    bool begin();

    // The main loop calls step() to compute the next state of the menu/display objects. 
    // 'dt' denotes the time passed since the last call to step(). A call to draw() draws 
    // all the display objects. So the game loop has to be implemented as follows
    //   while(true)
    //     step(dt);
    //     draw()    
    void step(unsigned long dt);
    void draw();

    // Add a MenuItem object to the internal list. Note that this object is "adopted" 
    // i.e. it is deleted by the dtor of class Menu.
    void addMenu(MenuItem* adoptMenuItem);
    
    // Add a DisplayObject object to the internal list. Note that this object is "adopted" 
    // i.e. it is deleted by the dtor of class Menu.
    // This is used to add additional visual elements to the screen, such as a battery symbol
    void addObject(DisplayObject* adoptObject);

    // A MenuItem might want to step/draw the additional visual elements
    void stepObjects(unsigned long dt); 
    void drawObjects();

    SoundEngine&     sound()  { return mySound;  }
    InputController& input()  { return myInput;  }
    fabgl::Canvas&   canvas() { return myCanvas; }

  private:
    void setActiveItem(int num);

  private:
    fabgl::Canvas&  myCanvas;
    SoundEngine     mySound;
    InputController myInput;

    std::vector<MenuItem*> myMenuItems;
    int myActiveItem;

    std::vector<DisplayObject*> myObjects;
};
