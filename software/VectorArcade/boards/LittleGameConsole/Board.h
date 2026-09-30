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
#define BOARD_NAME             "LittleGameConsole"

// ---- Display
#define BOARD_TFT_RESOLUTION   TFT_240x320
#define BOARD_TFT_ORIENTATION  fabgl::TFTOrientation::Rotate90
#define BOARD_TFT_SPI_HOST     HSPI_HOST
#define BOARD_TFT_INVOFF()     1   // 1: panel needs color inversion switched off

// ---- Features
#define BOARD_HAS_BACKLIGHT()    1   // cTFT_BL
#define BOARD_HAS_AUX_BUTTON()   1   // cPinBtnAux
#define BOARD_HAS_IMU()          0
#define BOARD_HAS_CHARGE_SENSE() 0   // cUSBStatus, cChargeStat
