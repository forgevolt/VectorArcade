#pragma once

#include <vector>
#include "DisplayObject.h"
#include "MathUtilities.h"
#include "Layout.h"
#include <SoundEngine.h>
#include "AsteroidsSounds.h"

class Ship;
class Shot; 
class Asteroid;
class Saucer;

// ---- Ship ------------------------------------------------------------------------------
// The hero of the game

class Ship : public DisplayObject
{
  public: 
    // If the ship explodes, then the explosion is visible for 5sec
    static constexpr unsigned long cTimeShipExplosionIsVisible = 5000;

  public:
    Ship(fabgl::Canvas& canvas, SoundEngine& sound, AsteroidsSounds& sounds);
    ~Ship() override {};

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;
    
    // Draws the shape of the ship. Required for the icon on the menu and the 
    // display of the number of ships left (number of lives)
    void drawShipShape(const Vector2& pos, float angle, float scale, const RGB888& color);

    // Reset to default values (e.g. no velocity)
    void reset(); 

    // Set/get position in screen coordinates
    void setPos(const Vector2& pos) { myPos = pos; }
    void setPos(float x, float y)   { myPos.x = x; myPos.y = y; }
    Vector2 getPos() const          { return myPos; }

    void setScale(float scale);

    // The ship can be rotated. Angle values are in degrees
    void setAngle(float angle); 
    float getAngle() const { return myAngle; }
    void changeAngle(float angle);

    // Thruster is on (true) or off (false)
    void thrust(bool isOn);

    // Randomly place the ship on a new location on the screen
    void hyperspace();
    bool isInHyperspace() const { return myHyperspaceActive; }

    // Returns true if the ship collides with an asteroid, a saucer or a shot (from a saucer)
    bool detectCollision(const Asteroid& a) const;
    bool detectCollision(const Saucer& s) const;
    bool detectCollision(const Shot& s) const;

    // Ship got hit by an asteroid/shot/saucer
    void gotHit();
    
  private:
    // Efficiently transform a coordinate 
    Vector2 rotateScaleTranslate(const Vector2& p) const;
    void computeRotateScaleConstants();
    float myCosScale; 
    float mySinScale; 

    SoundEngine& mySound;
    AsteroidsSounds& mySounds;

    Vector2 myPos;  
    Vector2 myVelocity;
    float myScale;        // Scale factor determines the size of the ship on screen
    float myAngle;        // Angle (orientation) of the ship in degrees
    float myAngleRad;     // Angle in radians
    bool  myThrusterIsOn; // true: thruster is on

    bool myExplosionInProgress; // Explosion animation is in progress
    unsigned long myStartExplosion;

    // After hyperspace is activated, the ship reappears after a delay
    bool myHyperspaceActive;
    unsigned long myStartHyperspace;

    // When ship is hit (asteroid, saucer or shot), then each line segment is part of the
    // debris and moves/rotates individually
    struct ExplosionDebris
    {
      int index;            // Index to the cShip array where the line data resides
      float angle;          // Angle in degrees
      float rotationSpeed;
      Vector2 pos;
      Vector2 velocity;
    };
    std::vector<ExplosionDebris> myExplosionDebris;
};


// ---- Shot ------------------------------------------------------------------------------
// The name says it all

class Shot : public DisplayObject
{
  public:
    Shot(fabgl::Canvas& canvas, bool isSaucerShot = false);
    ~Shot() override {};

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;
    
    // Reset to default values (e.g. no velocity)
    void reset();

    void fire(const Vector2& pos, const Vector2& direction);
    
    // Set/get position in screen coordinates
    void setPos(const Vector2& pos) { myPos = pos; }
    void setPos(float x, float y)   { myPos.x = x; myPos.y = y; }
    Vector2 getPos() const          { return myPos; }

    // Returns true if the asteroid/saucer is hit by the shot
    bool detectCollision(const Asteroid& a) const;
    bool detectCollision(const Saucer& s) const;

  private:
    bool myIsSaucerShot;
    Vector2 myPos;  
    Vector2 myVelocity;
    unsigned long myShotActiveTime;
};


// ---- Asteroid --------------------------------------------------------------------------

class Asteroid : public DisplayObject
{
  public:
    enum ESize
    {
      eSmall,
      eMedium,
      eLarge
    }; 

  public:
    Asteroid(fabgl::Canvas& canvas, SoundEngine& sound, AsteroidsSounds& sounds);
    ~Asteroid() override {};

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // Reset to default values (e.g. no velocity)
    void reset();

    // Change randomly to one of the predefined shapes 
    void newShape(ESize size);
    ESize getSize() const { return mySize; }

    // Set/get position in screen coordinates
    void setPos(const Vector2& pos) { myPos = pos; }
    void setPos(float x, float y)   { myPos.x = x; myPos.y = y; }
    Vector2 getPos() const          { return myPos; }

    // Set/get velocity
    void setVelocity(const Vector2& velocity) { myVelocity = velocity; }
    void setVelocity(float x, float y)        { myVelocity.x = x; myVelocity.y = y; } 
    Vector2 getVelocity() const               { return myVelocity; }

    // Returns true if point checkPoint is inside the polygon that forms the asteroid
    bool detectCollision(const Vector2& checkPoint) const;

    // Asteroid got hit and explodes
    void gotHit();

  private:
    SoundEngine& mySound;
    AsteroidsSounds& mySounds;

    Vector2 myPos;  
    Vector2 myVelocity;
    int myShape;
    ESize mySize;   // small, medium, large
    float myScale;  // Scale factor determines the size of the asteroid on screen

    bool myExplosionInProgress;       // Explosion animation is in progress
    unsigned long myStartExplosion;   // Time when the asteroid got hit
    std::vector<Vector2> myExplosion; // Holds (pos, vel)-pairs for the points of the explosion
};


// ---- Saucer ----------------------------------------------------------------------------
// Tries to shoot our hero. 

class Saucer : public DisplayObject
{
  public:
    enum ESize
    {
      eSmall,
      eBig
    }; 

  public:
    Saucer(fabgl::Canvas& canvas, SoundEngine& sound, AsteroidsSounds& sounds, const Ship& ship);
    ~Saucer() override;

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;
    
    // Reset to default values (e.g. no velocity)
    void reset(); 

    // Activate the saucer, small or big
    //   accuracy is in range [0, 1], the higher the value, the more precise the shots 
    //   of the small saucer
    void start(ESize size, float accuracy = 0.0);
    ESize getSize() const { return mySize; }

    // Returns true if point checkPoint is inside the shape of the saucer
    bool detectCollision(const Vector2& checkPoint) const;

    // Returns true if one of the shots fired by the saucer hits the ship
    bool detectCollisionWithShot(const Ship& ship) const;

    // Saucer got hit by an asteroid/shot
    void gotHit();
    
  private:
    SoundEngine& mySound;
    AsteroidsSounds& mySounds;
    const Ship& myShip; // Saucer is aware of the ship's position

    // Saucer keeps direction for cSegmentWidth pixels (x-direction)
    static constexpr int cSegmentWidth = Layout::cSaucerSegmentWidth;
    static constexpr int cNumSegments  = Layout::cSaucerNumSegments;

    Vector2 myPos;  
    Vector2 myVelocity[cNumSegments]; // cNumSegments*cSegmentWidth must exceed the screen width

    ESize mySize;               // small, big
    float myScale;              // Scale factor determines the size of the saucer on screen
    float myAccuracy;

    std::vector<Shot*> myShots; // Saucer may shoot a maximum of 5 shots
    long myShotCoolDownTime;    // Time between shots

    bool myExplosionInProgress; // Explosion animation is in progress
    unsigned long myStartExplosion;
    std::vector<Vector2> myExplosion; // Holds (pos, vel)-pairs for the points of the explosion
};
