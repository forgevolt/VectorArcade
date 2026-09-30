#pragma once

#include "MenuItem.h"
#include "Board.h"
#if BOARD_HAS_IMU()
#include <MPU6050_light.h>
#endif
#include "Camera.h"
#include "Eagle.h"
#include "LunarLanderSounds.h"
#include "LunarSurface.h"

// ---- LunarLander -----------------------------------------------------------------------
// Class implementing the game Lunar Lander:
//       https://en.wikipedia.org/wiki/Lunar_Lander_(1979_video_game)
// In order to start the game from the menu, the class is derived from MenuItem. 
// Game logic is implemented in the step() method that is called by the main loop of 
// the application.

class LunarLander : public MenuItem
{
  public:
#if BOARD_HAS_IMU()
    LunarLander(Menu& menu, MPU6050& imu);
#else
    LunarLander(Menu& menu);
#endif
    virtual ~LunarLander() {}

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // From class MenuItem
    void start() override;

  private:
    // Initialize the eagle, the moon surface and other variables ...
    void prepareStartGame(); // ... to get ready for a game
    void prepareAttempt();   // ... for the next landing attempt
    void prepareGameOver();  // ... for the game over screen

    enum EGameState
    {
      eSelectMode,          // user may select joy or gyro mode
      eSelectFuelAndStart,  // user selects amount of fuel, i.e. difficulty level
      eRunning,             // game is running
      eLanded,              // successful landing
      eCrashed,             // lander has crashed
      eOutOfFuel,           // lander has no fuel left 
      eGameOver             // game is over
    } 
    myState = eGameOver;

    // step() methods for each state
#if BOARD_HAS_IMU()
    void stepSelectMode(unsigned long dt);
#endif
    void stepSelectFuelAndStart(unsigned long dt);
    void stepRunning(unsigned long dt);
    void stepLanded(unsigned long dt);
    void stepCrashed(unsigned long dt);
    void stepOutOfFuel(unsigned long dt);
    void stepGameOver(unsigned long dt);

    // draw() methods for each state
#if BOARD_HAS_IMU()
    void drawSelectMode();
#endif
    void drawSelectFuelAndStart();
    void drawRunning();
    void drawLanded();
    void drawCrashed();
    void drawOutOfFuel();
    void drawGameOver();
    
    // Game information such as score, velocity, etc.
    void drawGameStats(); 

    // How good was the landing?
    enum ELandingType
    {
      eNone,
      eGreat,
      eGood,
      eMarooned
    } 
    myLandingType = eNone;

  private:
    InputController::State myInput;
#if BOARD_HAS_IMU()
    MPU6050& myIMU;
    bool myIsJoystickMode; // true: joystick, false: IMU
#endif

    LunarLanderSounds mySounds;
    Camera myCamera;
    Eagle myEagle;
    LunarSurface mySurface;

    // Game statistics
    unsigned long myStartAttemptTime = 0, myTimePassed = 0;
    unsigned long myStartMsgTime = 0;

    int myTotalScore = 0;
    int myScore = 0;
    float myAltitude = 0;
    float myFuelLost = 0;

    bool myIsZoomActive = false;      // If eagle is close to the surface during the descent, then we zoom in
    float myCameraXBeforeZoom = 0;    // Remember the x positions of camera and eagle in order to restore them
    float myEagleXBeforeZoom = 0;     // when zooming out
    unsigned long myZoomTime = 0;     // Hysteresis timer: do not toggle too fast between zoom in and out

    bool myShowInstructions = false;  // Game over screen toggles between the instruction screen and a simulated descent
};
