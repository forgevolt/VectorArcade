#pragma once

#include "MenuItem.h"
#include "MenuContainer.h"

// ---- MenuPage --------------------------------------------------------------------------
// Class MenuPage adds another menu level, e.g. with two levels:
//   Menu
//     Item 1
//     Item 2
//     MenuPage
//        Item 3

class MenuPage : public MenuItem
{
  public:
    MenuPage(Menu& menu, const String& name);
    ~MenuPage() override {}

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // From class MenuItem
    virtual void start();

    // Add a MenuItem object to the page. Note that this object is "adopted" 
    // i.e. it is deleted by the dtor of class MenuPage.
    void add(MenuItem* adoptMenuItem);

  private:
    MenuContainer myItems;
};
