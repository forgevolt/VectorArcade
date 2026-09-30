// Board definition - display setup and features of this console
#pragma once

#include "PinMap.h"

// Preprocessor flags rather than constexpr: they must be able to remove members, includes
// and whole blocks, and 'if constexpr' is C++17 while this core builds as gnu++11.
//
// The flags are function-like macros, tested as '#if BOARD_HAS_IMU()'. If Board.h is not
// included or a name is misspelled, the test fails to compile instead of silently
// evaluating to 0.

// ---- Identity
#define BOARD_NAME             "TintinRocketShooter"

// ---- Display
#define BOARD_TFT_RESOLUTION   TFT_240x240
#define BOARD_TFT_ORIENTATION  fabgl::TFTOrientation::Rotate0
#define BOARD_TFT_SPI_HOST     HSPI_HOST
#define BOARD_TFT_INVOFF()     0   // 1: panel needs color inversion switched off

// ---- Features
#define BOARD_HAS_BACKLIGHT()    0   // cTFT_BL
#define BOARD_HAS_AUX_BUTTON()   0   // cPinBtnAux
#define BOARD_HAS_IMU()          1
#define BOARD_HAS_CHARGE_SENSE() 1   // cUSBStatus, cChargeStat
