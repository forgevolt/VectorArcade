#include "DisplayObject.h"
#include <nvs_flash.h>
#include <Streaming.h>
#include "WipeAndReset.h"


// ---- WipeAndReset ----------------------------------------------------------------------

namespace
{

const String cMenuName = "Wipe and Reset";

} // namespace

// ----------------------------------------------------------------------------------------
WipeAndReset::WipeAndReset(Menu& menu)
: MenuItem(menu, cMenuName)
{}

// ----------------------------------------------------------------------------------------
void WipeAndReset::step(unsigned long dt)
{
  // ---- Process input events
  InputController::State s = myMenu.input().getState();

  if (s.a == true)
  {
    if (myIsYesSelected == true)
    {
      nvs_flash_erase(); // erase the NVS partition and...
      nvs_flash_init();  // initialize the NVS partition.

      // Restart ESP to reset the configuration values
      ESP.restart();
    }

    setState(eNotActive);
  }

  else if (s.right == true || s.left == true)
  {
    myIsYesSelected = !myIsYesSelected;
  }
}

// ----------------------------------------------------------------------------------------
void WipeAndReset::draw()
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
    
    canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
    canvas().setPenColor(cDefaultCol);
    canvas().selectFont(&fabgl::FONT_std_24);

    drawCenteredText(30, cMenuName);
    canvas().drawLine(0, 60, canvas().getWidth(), 60);
    drawCenteredText(80, "Reset to default");
    drawCenteredText(110, "settings?");

    drawButton(30,  180, 60, "Yes", myIsYesSelected);
    drawButton(canvas().getWidth()-30-60, 180, 60, "No",  !myIsYesSelected);
  }
}

// ----------------------------------------------------------------------------------------
void WipeAndReset::start()
{
  myIsYesSelected = false;
  setState(eActive);
}
