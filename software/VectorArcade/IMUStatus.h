#pragma once

#include "Board.h"
#if BOARD_HAS_IMU()

#include <MPU6050_light.h>
#include "DisplayObject.h"

// ---- IMUStatus -------------------------------------------------------------------------
// Displays the state of the IMU on the screen incl. the temperature.

class IMUStatus : public DisplayObject
{
  public:
    IMUStatus(fabgl::Canvas& canvas, MPU6050& imu);
    ~IMUStatus() override {};

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

  private:
    MPU6050& myIMU;

    // The temperature is read every 15sec, not in every call to draw()
    unsigned long myLastTempUpdate;
    float myTemp;
};

#endif
