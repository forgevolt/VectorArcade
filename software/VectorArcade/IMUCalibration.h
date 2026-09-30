#pragma once

#include "Board.h"
#if BOARD_HAS_IMU()

#include <MPU6050_light.h>
#include "MenuItem.h"

// ---- IMUCalibration --------------------------------------------------------------------
// Calculates the gyro and accelerometer offsets

class IMUCalibration : public MenuItem
{
  public:
    IMUCalibration(Menu& menu, MPU6050& imu);
    ~IMUCalibration() override {};

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // From MenuItem
    void start() override;

  private:
    void loadOffsets();
    void saveOffsets();

    MPU6050& myIMU;
    bool myIsCalibrationWaitPeriodActive;
    unsigned long myCalibrationStartTime = 0;
};

#endif
