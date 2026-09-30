// ---------------------------------------------------------------------------------------------
// VectorArcade - Asteroids and Lunar Lander, the 1979 vector arcade classics, on a small
//                ESP32 game console with a TFT display, an analog joystick and buttons.
//                One code base for two boards, selected in BoardSelect.h:
//                  LittleGameConsole   - 2.8" 320x240 display, five buttons
//                  TintinRocketShooter - 240x240 display in a Tintin rocket, four buttons and an IMU
//           
// Christoph Streit - 2025-2026
// ---------------------------------------------------------------------------------------------

// #pragma GCC optimize ("O2")

#include <Streaming.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <LittleFS.h>
#include <esp_partition.h>
#include <sstream>
#include <iomanip>

// ---- Gauge

#include "Adafruit_MAX1704X.h"
#include "BatteryStatus.h"

Adafruit_MAX17048 fuelGauge;

// ---- Display

#include "fabgl.h"
#include "Board.h"
#include "Layout.h"
#include "Version.h"
#include "SoundClips.h"

#pragma message("VectorArcade " VECTORARCADE_VERSION ": building for " BOARD_NAME ", sounds " SOUNDS_SOURCE)

#if BOARD_TFT_INVOFF()
// FabGL inverts the colors by default -> switch inversion off (INVOFF)
#ifndef ST7789_INVOFF
#define ST7789_INVOFF     0x20
#endif

class ST7789ControllerINVOFF : public fabgl::ST7789Controller {
protected:
  void softReset() override 
  {
    fabgl::ST7789Controller::softReset();
    SPIBeginWrite();
    writeCommand(ST7789_INVOFF);
    SPIEndWrite();
  }
};

ST7789ControllerINVOFF DisplayController;
#else
fabgl::ST7789Controller DisplayController;
#endif
fabgl::Canvas canvas(&DisplayController);

// ---- Menu

#include "Menu.h"
#include "MenuItem.h"
#include "MenuPage.h"
#include "FPSToggle.h"
#include "SoundVolume.h"
#include "JoystickCalibration.h"
#include "WipeAndReset.h"
#include "LunarLander.h"
#include "Asteroids.h"

Menu m(canvas);
FPSToggle* fpsToggle = nullptr; // Owned by the menu

#if BOARD_HAS_IMU()
// ---- IMU

#include <Wire.h>
#include <MPU6050_light.h>
#include "IMUStatus.h"
#include "IMUCalibration.h"

MPU6050 imu(Wire);
#endif

//----------------------------------------------------------------------------------------------
void setup()
{
#if BOARD_HAS_BACKLIGHT()
  pinMode(cTFT_BL, OUTPUT);
  digitalWrite(cTFT_BL, LOW); // Backlight off
#endif

  Serial.begin(115200); // Used for info/debug

  // On a classic ESP32 Serial is a UART and is ready as soon as begin() returns. With native
  // USB (e.g. ESP32-S3 USB CDC) it only becomes true once a host has opened the port: wait
  // briefly so the startup messages are not lost, but never block boot without a host.
  const unsigned long serialWaitStart = millis();
  while (!Serial && millis() - serialWaitStart < 1000)
  {
    delay(10);
  }

  Serial << "VectorArcade " << VECTORARCADE_VERSION << " for " << BOARD_NAME << endl;
  Serial << "Built " << __DATE__ << " " << __TIME__ << endl;
  Serial << "Sounds " << SOUNDS_SOURCE << endl;

  WiFi.mode(WIFI_OFF);
  Wire.begin(cI2C_SDA, cI2C_SCL);

  // ---- Display

  DisplayController.begin(cSPI_SCK, cSPI_MOSI, cTFT_DC, cTFT_RESET, -1, BOARD_TFT_SPI_HOST);
  DisplayController.setResolution(BOARD_TFT_RESOLUTION, -1, -1, true);
  DisplayController.setOrientation(BOARD_TFT_ORIENTATION);

  // Clear screen
  canvas.setBrushColor(Color::Black);
  canvas.clear();
  canvas.swapBuffers();

#if BOARD_HAS_BACKLIGHT()
  digitalWrite(cTFT_BL, HIGH); // Backlight on
#endif

  // ---- Gauge 

  if (fuelGauge.begin() == false) 
  {
    Serial << "Couldn't find MAX17048\nMake sure a battery is plugged in!" << endl;
  }
  delay(500);
  // fuelGauge.quickStart();

#if BOARD_HAS_IMU()
  // ---- IMU

  byte imuStatus = imu.begin();
  if (imuStatus != 0)
  {
    Serial << "Couldn't find MPU6050 (status " << imuStatus << ")" << endl;
  }

#endif
#if SOUNDS_FROM_LITTLEFS()
  // ---- Filesystem (sound clips)

  // The partition labelled "littlefs" (partitions.csv) holds a LittleFS image. Its subtype is
  // still "spiffs": ESP-IDF 4.4 has no LittleFS subtype, and LittleFS finds the partition by
  // label. begin(false) never formats on its own: a partition that cannot be mounted (never
  // formatted, or another filesystem) is formatted here, explicitly and reported. A missing
  // partition (wrong partition table) is only reported.
  const char* cFSPartition = "littlefs";

  if (esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, cFSPartition) == nullptr)
  {
    Serial << "LittleFS: no partition \"" << cFSPartition << "\" - check partitions.csv" << endl;
  }
  else if (LittleFS.begin(false, "/littlefs", 10, cFSPartition) == false)
  {
    Serial << "LittleFS: partition cannot be mounted - formatting it as LittleFS" << endl;
    if (LittleFS.format() == true && LittleFS.begin(false, "/littlefs", 10, cFSPartition) == true)
    {
      Serial << "LittleFS: formatted and mounted, " << LittleFS.totalBytes() << " bytes" << endl;
    }
    else
    {
      Serial << "LittleFS: formatting failed" << endl;
    }
  }
  checkSoundFiles();
#endif

  // ---- Menu

#if BOARD_HAS_IMU()
  m.addMenu(new LunarLander(m, imu));
#else
  m.addMenu(new LunarLander(m));
#endif
  m.addMenu(new Asteroids(m));

  MenuPage* p = new MenuPage(m, "Configuration");
    p->add(new JoystickCalibration(m));
#if BOARD_HAS_IMU()
    p->add(new IMUCalibration(m, imu));
#endif
    p->add(new SoundVolume(m));
    p->add(new WipeAndReset(m));
  m.addMenu(p);

  fpsToggle = new FPSToggle(m);
  m.addMenu(fpsToggle);
  m.addObject(new BatteryStatus(canvas, fuelGauge));
#if BOARD_HAS_IMU()
  m.addObject(new IMUStatus(canvas, imu));
#endif
  m.begin();
}

//----------------------------------------------------------------------------------------------
void loop()
{
  // Animation steps: at most "targetFPS" frames per second are rendered
  const unsigned long targetFPS = 25;
  const unsigned long dt = 1000 / targetFPS;

  int64_t t0, t1; // To compute the rendering time of one frame
  int delayMS;    // and to delay if we are too fast

  while (true)
  {
    // ---- Compute FPS each second

    static int64_t stime  = esp_timer_get_time();
    static int FPS        = 0;
    static int FPSCounter = 0;

    if (esp_timer_get_time() - stime > 1000000) 
    {
      FPS = FPSCounter;
      stime = esp_timer_get_time();
      FPSCounter = 0;
    }
    ++FPSCounter;

    // ---- Update frame and redraw

    t0 = esp_timer_get_time();  // us

    m.step(dt);
    m.draw();
    
    if (fpsToggle->isOn())
    {
      canvas.setGlyphOptions(GlyphOptions().FillBackground(true));
      canvas.setPenColor(cDefaultCol);
      canvas.setBrushColor(Color::Black);    
      canvas.selectFont(&fabgl::FONT_6x8);
      canvas.drawTextFmt(Layout::cFPSPosX, Layout::cFPSPosY, " %d FPS ", FPS);
    }

    canvas.swapBuffers();

    t1 = esp_timer_get_time();  // us
    delayMS = (dt - (t1 - t0) / 1000);
    if (delayMS > 0)
      vTaskDelay(delayMS / portTICK_PERIOD_MS);
  }
}

