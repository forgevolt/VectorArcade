#include "DisplayObject.h"
#include <Preferences.h>
#include <Streaming.h>
#include "SoundVolume.h"


// ---- SoundVolume -----------------------------------------------------------------------

namespace
{

const String cMenuName = "Sound Volume";

} // namespace

// ----------------------------------------------------------------------------------------
SoundVolume::SoundVolume(Menu& menu)
: MenuItem(menu, cMenuName)
{
  loadVolume();
}

// ----------------------------------------------------------------------------------------
void SoundVolume::step(unsigned long dt)
{
  // ---- Process input events
  InputController::State s = myMenu.input().getState();

  if (s.a || s.b || s.sel || s.start || s.joy)
  {
    sound().play(sound().click());
    saveVolume();
    setState(eNotActive);
  }

  else if (s.right == true)
  {
    sound().setVolume(sound().getVolume() + 10);
    sound().play(sound().beep());
  }

  else if (s.left == true)
  {
    sound().setVolume(sound().getVolume() - 10);
    sound().play(sound().beep());
  }
}

// ----------------------------------------------------------------------------------------
void SoundVolume::draw()
{
  if (isActive() == false)
  {
    MenuItem::draw();
  }
  else
  {
    // Clear screen
    canvas().setBrushColor(Color::Black);
    canvas().clear();

    drawHeader(30, cMenuName);
    drawCenteredText(80, "Move joystick left/right");
    drawCenteredText(110, "to change volume.");
    drawCenteredText(150, "Press button to exit.");

    // Visualize sound volume with a "progress bar" 
    drawFrame(29, 200-10, canvas().getWidth()-30, 200+11);
    canvas().setBrushColor(cBarCol);
    canvas().fillRectangle(33, 200-6, 33+float(canvas().getWidth()-66)/200.0f*sound().getVolume(), 200+7);
  }
}

// ----------------------------------------------------------------------------------------
void SoundVolume::start()
{
  setState(eActive);
}

// ----------------------------------------------------------------------------------------
void SoundVolume::loadVolume()
{
  // Read volume from NVS or use the default value (percent)
  Preferences p;
  p.begin("Sound");
    sound().setVolume(p.getInt("volumePct", 100));
  p.end();
}

// ----------------------------------------------------------------------------------------
void SoundVolume::saveVolume()
{
  Preferences p;
  p.begin("Sound");
    p.putInt("volumePct", sound().getVolume());
  p.end();
}
