#include "FPSToggle.h"
#include <Preferences.h>

// ---- FPSToggle -------------------------------------------------------------------------

namespace
{

const String cMenuNameOn  = "Show FPS";
const String cMenuNameOff = "Hide FPS";

constexpr const char* cPreferencesNS = "FPS";
constexpr const char* cKey           = "showFPS";

} // namespace

// ----------------------------------------------------------------------------------------
FPSToggle::FPSToggle(Menu& menu)
: MenuItem(menu, "")
{
  // Read FPS configuration from preferences
  Preferences p;
  p.begin(cPreferencesNS);
  myShowFPS = p.getBool(cKey, true);
  p.end();

  myMenuName = myShowFPS ? cMenuNameOff : cMenuNameOn;
}

// ----------------------------------------------------------------------------------------
void FPSToggle::start()
{
  myShowFPS = !myShowFPS;

  // Set FPS configuration
  Preferences p;
  p.begin(cPreferencesNS);
  p.putBool(cKey, myShowFPS);
  p.end();

  myMenuName = myShowFPS ? cMenuNameOff : cMenuNameOn;
}
