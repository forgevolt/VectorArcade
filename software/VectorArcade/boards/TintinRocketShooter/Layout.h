// Layout - screen geometry of this console
#pragma once

// The values are chosen for the physical size of this panel and for legibility. They are not
// derived from the resolution.

namespace Layout
{
  // ---- Main loop
  constexpr int cFPSPosX = 190;  // FPS counter
  constexpr int cFPSPosY = 232;

  // ---- Battery icon
  constexpr int  cBatteryPosX               = 10;
  constexpr int  cBatteryPosY               = 3;
  constexpr int  cBatteryWidth              = 39;
  constexpr int  cBatteryHeight             = 20;
  constexpr int  cBatterySegmentWidth       = 7;   // One segment per 20% charge
  constexpr int  cBatteryTerminalWidth      = 3;   // Battery tip
  constexpr int  cBatteryTerminalHalfHeight = 5;
  constexpr bool cBatteryDoubleOutline      = true;

  // ---- Lunar Lander
  constexpr float cLunarZoom        = 1.3f; // Chosen for legibility on the small panel
  constexpr float cLunarMaxWorldY   = 247;   // Lowest point (world coordinates)
  constexpr float cLunarStartEagleY = 122;   // Eagle on the select screens
  constexpr int   cLunarStatsLeft   = 7;    // Game statistics columns
  constexpr int   cLunarStatsRight  = 135;

  // ---- Asteroids
  constexpr int cSaucerSegmentWidth = 40;  // Saucer keeps its direction for this many pixels (x)
  constexpr int cSaucerNumSegments  = 7;   // cSaucerNumSegments*cSaucerSegmentWidth must exceed the screen width
  constexpr int cShipShotRange      = 150; // Pixels a shot travels before it disappears
  constexpr int cSaucerShotRange    = 110;
}
