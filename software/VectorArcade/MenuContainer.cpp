#include <Streaming.h>
#include "MenuContainer.h"
#include "MenuItem.h"

// ---- MenuContainer ---------------------------------------------------------------------

namespace
{

// Layout of the list (pixels): y position of the first item and distance between items
const int16_t cFirstItemPosY = 60;
const int16_t cItemSpacing   = 45;

} // namespace

// ----------------------------------------------------------------------------------------
MenuContainer::MenuContainer(SoundEngine& sound)
: mySound(sound)
{}

// ----------------------------------------------------------------------------------------
MenuContainer::~MenuContainer()
{
  for (auto item : myItems)
  {
    delete item;
  }
}

// ----------------------------------------------------------------------------------------
void MenuContainer::add(MenuItem* adoptMenuItem)
{
  if (adoptMenuItem == nullptr)
  {
    Serial << __PRETTY_FUNCTION__  << " -> adoptMenuItem == nullptr" << endl;
    return;
  }

  myItems.push_back(adoptMenuItem);
}

// ----------------------------------------------------------------------------------------
void MenuContainer::reset()
{
  int16_t y = cFirstItemPosY;
  for (auto item : myItems)
  {
    item->setPosY(y);
    item->setSelected(false);
    item->setState(DisplayObject::eNotActive);
    y += cItemSpacing;
  }

  setActiveItem(0);
}

// ----------------------------------------------------------------------------------------
bool MenuContainer::isItemRunning() const
{
  return myItems.empty() == false && myItems[myActiveItem]->isActive() == true;
}

// ----------------------------------------------------------------------------------------
void MenuContainer::stepRunningItem(unsigned long dt)
{
  myItems[myActiveItem]->step(dt);
}

// ----------------------------------------------------------------------------------------
void MenuContainer::drawRunningItem()
{
  myItems[myActiveItem]->draw();
}

// ----------------------------------------------------------------------------------------
bool MenuContainer::navigate(const InputController::State& s)
{
  if (myItems.empty() == true)
    return false;

  if (s.start == true || s.joy == true || s.a == true)
  {
    // Start processing the MenuItem
    mySound.play(mySound.click());
    myItems[myActiveItem]->start();
    return true;
  }

  if (s.down == true)
    setActiveItem(myActiveItem+1);

  if (s.up == true)
    setActiveItem(myActiveItem-1);

  return false;
}

// ----------------------------------------------------------------------------------------
void MenuContainer::drawList(fabgl::Canvas& canvas)
{
  // Clear screen
  canvas.setBrushColor(Color::Black);
  canvas.clear();
  canvas.setGlyphOptions(GlyphOptions().FillBackground(false));

  for (auto item : myItems)
  {
    item->draw();
  }
}

// ----------------------------------------------------------------------------------------
void MenuContainer::setActiveItem(int num)
{
  if (myItems.empty() == true)
    return;

  // Processing of MenuItem is already in progress -> do not change active MenuItem
  if (myItems[myActiveItem]->isActive() == true)
    return;

  if (num < 0)
    num = myItems.size()-1;
  else if (num >= (int)myItems.size())
    num = 0;

  myItems[myActiveItem]->setSelected(false);
  myItems[num]->setSelected(true);

  myActiveItem = num;
}
