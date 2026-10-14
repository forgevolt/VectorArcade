// Layout - screen geometry of this console
#pragma once

// The values are chosen for the physical size of this panel and for legibility. They are not
// derived from the resolution.

namespace Layout
{
  // ---- Main loop
  constexpr int cFPSPosX = 270;  // FPS counter
  constexpr int cFPSPosY = 231;

  // ---- Battery icon
  constexpr int  cBatteryPosX               = 20;
  constexpr int  cBatteryPosY               = 5;
  constexpr int  cBatteryWidth              = 29;
  constexpr int  cBatteryHeight             = 18;
  constexpr int  cBatterySegmentWidth       = 5;   // One segment per 20% charge
  constexpr int  cBatteryTerminalWidth      = 2;   // Battery tip
  constexpr int  cBatteryTerminalHalfHeight = 4;
  constexpr bool cBatteryDoubleOutline      = false;

  // ---- Lunar Lander
  constexpr float cLunarZoom        = 320.0f / 240.0f; // The whole lunar surface (240 wide) fits the screen
  constexpr float cLunarMaxWorldY   = 245;   // Lowest point (world coordinates)
  constexpr float cLunarStartEagleY = 102;   // Eagle on the select screens
  constexpr int   cLunarStatsLeft   = 25;    // Game statistics columns
  constexpr int   cLunarStatsRight  = 197;

  // ---- Asteroids
  constexpr int cSaucerSegmentWidth = 50;  // Saucer keeps its direction for this many pixels (x)
  constexpr int cSaucerNumSegments  = 8;   // cSaucerNumSegments*cSaucerSegmentWidth must exceed the screen width
  constexpr int cShipShotRange      = 180; // Pixels a shot travels before it disappears
  constexpr int cSaucerShotRange    = 150;
}
