// Theme.h - colors of the user interface, the same on both consoles
#pragma once

#include <fabgl.h>

// FabGL's named colors (Color::Black, Color::BrightWhite, ...) are used directly where they
// fit. The colors below have a role on screen.

// ---- General
const RGB888 cDefaultCol  (170, 170, 170);  // Text, frames, lines and vector shapes
const RGB888 cShadowCol   ( 80,  80,  80);  // Shadow of the buttons in the instruction screens
const RGB888 cBarCol      (  0,   0, 255);  // Fill of the volume and time bars, outline of the joystick range
const RGB888 cRangeCol    (  0,   0, 140);  // Range covered in the joystick calibration
const RGB888 cConfirmCol  (  0, 200,   0);  // Joystick calibration: release measured, edge reached
const RGB888 cLetGoCol    ( 74, 150, 222);  // Joystick calibration: stick pushed, now let go
const RGB888 cHoldStillCol(255, 180,   0);  // Joystick calibration: stick released, hold still
const RGB888 cMissedCol   (255, 180,   0);  // Joystick calibration: edge not reached, calibration not stored
const RGB888 cIMUSpotCol  ( 71, 212,  90);  // Tilt indicator of the IMU status

// ---- Battery charge levels
const RGB888 cBattery80_100Col ( 71, 212, 90);
const RGB888 cBattery60_80Col  (153, 217, 74);
const RGB888 cBattery40_60Col  (255, 212,  5);
const RGB888 cBattery20_40Col  (233, 113, 50);
const RGB888 cBattery00_20Col  (144,   0,  0);

// ---- Lunar Lander
const RGB888 cLandingZoneCol (210, 210, 210);  // Bonus landing zones and their labels
const RGB888 cStarCol        ( 80,  80,  80);
