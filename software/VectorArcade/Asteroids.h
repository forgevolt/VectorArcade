#pragma once

#include "MenuItem.h"
#include "AsteroidsGameObjects.h"

// ---- Asteroids -------------------------------------------------------------------------
// Class implementing the game Asteroids:
//       https://en.wikipedia.org/wiki/Asteroids_(video_game)
// In order to start the game from the menu, the class is derived from MenuItem. 
// Game logic is implemented in the step() method that is called by the main loop of 
// the application.

class Asteroids : public MenuItem
{
  public:
    Asteroids(Menu& menu);
    ~Asteroids() override;

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // From class MenuItem
    void start() override;

  private:
    // Initialize variables ...
    void prepareSplashScreen();        // ... for the splash screen
    void prepareAttempt(bool newGame); // ... for the next attempt
 
    // Find an Asteroid object that is not in use yet (state == eNotActive). 
    // If none is available, then create one and add it to the myAsteroids vector<>.  
    Asteroid* getNotActiveAsteroid();

    // Split an asteroid into two smaller pieces
    void splitAsteroid(Asteroid& astHit);

    // Returns true if the area at pos (i.e. the new ship's position) is safe, 
    // i.e. not covered by an asteroid, and no nasty saucer is active
    bool isAreaSafe(const Vector2& pos);

    enum EGameState
    {
      eSplashScreen,   // splash screen (startup)
      eRunning,        // game is running
      eLevelCleared,   // destroyed all asteroids
      eGotHit,         // ship was hit by an asteroid or saucer
      eGameOver        // game is over
    } 
    myState;

    // step() methods for each state
    void stepSplashScreen(unsigned long dt);
    void stepRunning(unsigned long dt);
    void stepLevelCleared(unsigned long dt);
    void stepGotHit(unsigned long dt);
    void stepGameOver(unsigned long dt);

    // draw() methods for each state
    void drawSplashScreen();
    void drawRunning();
    void drawLevelCleared();
    void drawGotHit();
    void drawGameOver();

    // Game information such as score, number of lives, etc.
    void drawGameStats(); 

  private:
    InputController::State myInput;
    AsteroidsSounds mySounds;
    Ship myShip;
    std::vector<Shot*> myShots;
    std::vector<Asteroid*> myAsteroids;
    Saucer mySaucer;

    int myScore;
    int myHighScore;
    int myNumLives;
    int myLevel;
    float mySaucerAccuracy;

    unsigned long myStartWaitTime;
    unsigned long myTimeSaucerAppeared; // Time (millis()) when saucer appeared the last time

    int myNumAsteroidsToShoot;
    int myNumAsteroidsHit;

    bool myShowInstructions;  // Toggles between the instruction screen and a splash screen bitmap
};
