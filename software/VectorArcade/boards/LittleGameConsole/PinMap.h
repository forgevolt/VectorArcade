// Single source of truth for every GPIO used in the project
#pragma once

// Strapping pins - the ESP32 latches GPIO 0, 2, 5, 12 and 15 at reset. This board uses four:
//
// GPIO 12 (cSPI_MOSI) is the MTDI strapping pin: its level is latched at reset and selects the
// flash voltage, where high means 1.8 V. With 3.3 V flash the board will not boot if this pin
// is high at reset. Safe here because SPI MOSI idles low. Do not fit a pull-up to this line.
//
// GPIO 2 (cTFT_BL) must be low or floating at reset for the serial bootloader to be entered,
// so a backlight circuit that pulls it high can break flashing. setup() drives it low before
// anything else runs. Do not fit a pull-up to this line.
//
// GPIO 15 (cTFT_DC) is the MTDO strapping pin (boot log and SDIO timing) - harmless here.
// GPIO 5 (cI2S_BCLK) is a strapping pin (SDIO timing) - harmless here.

// SPI (display)
constexpr int cSPI_SCK   = 14;  // SPI clock pin
constexpr int cSPI_MISO  = -1;  // Not used
constexpr int cSPI_MOSI  = 12;  // SPI Microcontroller Out Serial In pin (often named SDA)

// Display
constexpr int cTFT_DC    = 15;  // SPI data or command selector pin
constexpr int cTFT_RESET = 13;  // TFT reset pin
constexpr int cTFT_BL    = 2;   // TFT backlight pin

// I2C (fuel gauge)
constexpr int cI2C_SDA   = 22;
constexpr int cI2C_SCL   = 21;

// I2S (audio)
constexpr int cI2S_DOUT  = 8;   // I2S data out pin
constexpr int cI2S_BCLK  = 5;   // Bit clock
constexpr int cI2S_LRC   = 19;  // Left/Right clock, also known as Frame clock or word select

// Buttons
constexpr int cPinBtnA     = 26;
constexpr int cPinBtnB     = 25;
constexpr int cPinBtnAux   = 4;
constexpr int cPinBtnSel   = 7;
constexpr int cPinBtnStart = 20;

// Joystick
constexpr int cPinJoyBtn = 33;
constexpr int cPinJoyX   = 37;
constexpr int cPinJoyY   = 38;
