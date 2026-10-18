#pragma once

#include <vector>
#include <fabgl.h>
#include <SoundEngine.h>
#include "InputController.h"

// ---- MenuContainer ---------------------------------------------------------------------
// The list of MenuItems shared by Menu and MenuPage: adopting the items, laying them out,
// moving the selection and starting the selected item.

class MenuItem;

class MenuContainer
{
  public:
    explicit MenuContainer(SoundEngine& sound);
    ~MenuContainer();

    MenuContainer(const MenuContainer&) = delete;
    MenuContainer& operator=(const MenuContainer&) = delete;

    // Add a MenuItem object to the list. Note that this object is "adopted" i.e. it is
    // deleted by the dtor of class MenuContainer.
    void add(MenuItem* adoptMenuItem);

    // Positions the items, deselects them and selects the first one
    void reset();

    // true while the selected item runs (a game or a configuration screen)
    bool isItemRunning() const;
    void stepRunningItem(unsigned long dt);
    void drawRunningItem();

    // Ends the running item without its own exit; its start() sets it up afresh
    void stopRunningItem();

    // Start, A or the joystick button starts the selected item, up/down move the selection.
    // Returns true if an item was started.
    bool navigate(const InputController::State& s);

    // Clears the screen and draws the list
    void drawList(fabgl::Canvas& canvas);

  private:
    void setActiveItem(int num);

    SoundEngine&           mySound;
    std::vector<MenuItem*> myItems;
    int                    myActiveItem = 0;
};
