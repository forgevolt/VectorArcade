#pragma once

#include <vector>
#include "DisplayObject.h"
#include "MathUtilities.h"
#include "Camera.h"
#include <SoundEngine.h>
#include "LunarLanderSounds.h"

// ---- Eagle -----------------------------------------------------------------------------
// The moon lander Eagle

class Eagle : public DisplayObject
{
  public:
    static constexpr float cGravity       = 1.62;  // moon gravity in m/s^2
    static constexpr float cStartFuel     = 750;   // measured in l, = fuel increments also (same as Arcade)
    static constexpr float cFuelLow       = 100;   // measured in l (same as Arcade)
    static constexpr float cMaxFuel       = 9999;  // measured in l (same as Arcade)
    static constexpr float cFuelBurnRate  = 10;    // litres/sec -> 75s burn time with full thrust
    static constexpr float cMaxThrust     = 5;     // measured in m/s^2

  public:
    Eagle(fabgl::Canvas& canvas, Camera& camera, SoundEngine& sound, LunarLanderSounds& sounds);
    ~Eagle() override {};

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // Draws the upright shape of the lander at a screen position, independent of the state
    // of the lander and of the camera. Required for the icon on the menu.
    void drawShape(const Vector2& screenPos, float scale, const RGB888& color);
    
    // Reset to default values (e.g. no velocity). If 'resetFuel' is false, the fuel 
    // remains unchanged 
    void reset(bool resetFuel); 

    // Eagle has landed
    void hasLanded();

    // Eagle is lost
    void hasCrashed();

    // Trigger abort maneuver, i.e.
    //   Rotate to upright position
    //   Extra thrust to escape 
    void abort();
    void stopAbort();

    void setColor(const RGB888& color);
 
    void setDebug(bool isOn) { myIsDebugOn = isOn; }
    
    // Set/get position in world coordinates
    void setPos(const Vector2& pos)   { myPos = pos; }
    void setPos(float x, float y)     { myPos.x = x; myPos.y = y; }
    const Vector2& getPos() const     { return myPos; }

    // Set/get velocity in m/s
    void setVelocity(const Vector2& velocity) { myVelocity = velocity; }
    void setVelocity(float x, float y)        { myVelocity.x = x; myVelocity.y = y; } 
    const Vector2& getVelocity() const        { return myVelocity; }

    // Access bounding box, e.g. to check for a collision
    const Vector2& getBBMin() const { return myBBMin; }
    const Vector2& getBBMax() const { return myBBMax; }

    // Thrust is in range [0, cMaxThrust] m/s^2
    void applyThrust(float thrust);

    void setScale(float scale);

    // Range is [-90, 90] in degrees
    void setAngle(float angle); 
    float getAngle() const { return myAngle; }

    void setFuel(float fuel);
    float getFuel() const { return myFuel; }

  private:
    void playRocketSound();

    // Efficiently transform a coordinate 
    Vector2 rotateScale(float x, float y);
    Vector2 rotateScaleTranslate(float x, float y);
    void computeRotateScaleConstants();
    float myCosScale; 
    float mySinScale; 

    // The bounding box 
    void computeBoundingBox();
    Vector2 myBBMin, myBBMax;
    
    // Helpers to make use of class Vector2
    void moveTo(const Vector2& p);
    void lineTo(const Vector2& p);
    void drawLine(const Vector2& p1, const Vector2& p2);

    RGB888 myColor;  
    bool myIsDebugOn;

    Camera& myCamera;
    SoundEngine& mySound;
    LunarLanderSounds& mySounds;
    int myRocketVolume;  // Current volume of the rocket engine in %

    Vector2 myPos;  
    Vector2 myVelocity;  // Measured in m/s
    float myScale;       // Scale factor determines the size of the eagle on screen
    float myAngle;       // Angle (orientation) of lander in degrees
    float myAngleRad;    // Angle in radians
    float myThrust;      // Measured in m/s^2, range [0, cMaxThrust]
    float myFuel;        // Measured in l

    float myAbortFuelLevel;     // Fuel level when abort maneuver was started
    bool myAbortInProgress;     // While abort maneuver is in progress, no changes to angle or thrust are accepted

    bool myExplosionInProgress; // Explosion animation is in progress

    // When lander has crashed, then each line segment is part of the debris and moves/rotates individually
    struct ExplosionDebris
    {
      int index;            // Index to the cEagle array where the line data resides
      float angle;          // Angle in degrees
      float rotationSpeed;
      Vector2 pos;
      Vector2 velocity;
    };
    std::vector<ExplosionDebris> myExplosionDebris;
};
