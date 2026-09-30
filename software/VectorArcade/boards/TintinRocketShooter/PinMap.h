// Single source of truth for every GPIO used in the project
#pragma once

// Strapping pins - the ESP32 latches GPIO 0, 2, 5, 12 and 15 at reset. This board uses three:
//
// GPIO 12 (cSPI_MOSI) is the MTDI strapping pin: its level is latched at reset and selects the
// flash voltage, where high means 1.8 V. With 3.3 V flash the board will not boot if this pin
// is high at reset. Safe here because SPI MOSI idles low. Do not fit a pull-up to this line.
//
// GPIO 15 (cTFT_DC) is the MTDO strapping pin (boot log and SDIO timing) - harmless here.
// GPIO 5 (cI2S_BCLK) is a strapping pin (SDIO timing) - harmless here.
//
// Input-only pins - GPIO 34 to 39 have no internal pull-up or pull-down, so INPUT_PULLUP has
// no effect on them. cPinBtnSel (36) and cPinJoyBtn (38) rely on external pull-ups.

// SPI (display)
constexpr int cSPI_SCK   = 14;  // SPI clock pin
constexpr int cSPI_MISO  = -1;  // Not used
constexpr int cSPI_MOSI  = 12;  // SPI Microcontroller Out Serial In pin (often named SDA)

// Display
constexpr int cTFT_DC    = 15;  // SPI data or command selector pin
constexpr int cTFT_RESET = 13;  // TFT reset pin

// I2C (fuel gauge and IMU)
constexpr int cI2C_SDA   = 22;
constexpr int cI2C_SCL   = 21;

// I2S (audio)
constexpr int cI2S_DOUT  = 8;   // I2S data out pin
constexpr int cI2S_BCLK  = 5;   // Bit clock
constexpr int cI2S_LRC   = 19;  // Left/Right clock, also known as Frame clock or word select

// Power sense
constexpr int cUSBStatus  = 26; // The USB pin indicates if we are USB powered
constexpr int cChargeStat = 25; // The chargeStat pin indicates the current charge mode

// Buttons
constexpr int cPinBtnA     = 4;
constexpr int cPinBtnB     = 7;
constexpr int cPinBtnSel   = 36;
constexpr int cPinBtnStart = 20;

// Joystick
constexpr int cPinJoyBtn = 38;
constexpr int cPinJoyX   = 32;
constexpr int cPinJoyY   = 37;
