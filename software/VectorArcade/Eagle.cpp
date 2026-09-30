#include "Eagle.h"
#include "LunarLanderSounds.h"
#include <vector>
#include <Streaming.h>

// ---- Eagle -----------------------------------------------------------------------------

namespace
{

// moveTo, lineTo pairs
constexpr float cEagle[] = {
  
  // Octagon
  -0.309,  0.038, -0.563, -0.217, 
  -0.563, -0.217, -0.563, -0.683,
  -0.563, -0.683, -0.233, -1.013,
  -0.233, -1.013,  0.233, -1.013,
   0.233, -1.013,  0.563, -0.683,
   0.563, -0.683,  0.563, -0.217,
   0.563, -0.217,  0.309,  0.038,
/*
  Removed, too small, almost not visible
  
  // Side thrusters
  -0.563, -0.351, -0.649, -0.302,
  -0.649, -0.302, -0.649, -0.598,
  -0.649, -0.598, -0.563, -0.549,

   0.563, -0.351,  0.649, -0.302,
   0.649, -0.302,  0.649, -0.598,
   0.649, -0.598,  0.563, -0.549,
*/
  // Rectangle base
  -0.610,  0.404, -0.610,  0.038,
  -0.610,  0.038,  0.610,  0.038,
   0.610,  0.038,  0.610,  0.404,
   0.610,  0.404, -0.610,  0.404,

  // Rocket nozzle
  -0.233,  0.404, -0.433,  0.770,
  -0.433,  0.770,  0.433,  0.770,
   0.433,  0.770,  0.233,  0.404,

  // Legs
  -0.610,  0.404, -0.958, 1.013,
   0.610,  0.404,  0.958, 1.013,

  // Landing pads
  -1.080,  1.013, -0.837, 1.013,
   1.080,  1.013,  0.837, 1.013
};

} // namespace


// ----------------------------------------------------------------------------------------
Eagle::Eagle(fabgl::Canvas& canvas, Camera& camera, SoundEngine& sound, LunarLanderSounds& sounds)
: DisplayObject(canvas),
  myColor(cDefaultCol),
  myIsDebugOn(false),
  myCamera(camera),
  mySound(sound),
  mySounds(sounds),
  myRocketVolume(10)
{
  reset(true);
  computeRotateScaleConstants();
  computeBoundingBox();

  // Initialize explosion debris objects
  ExplosionDebris ed;

  for (size_t i=0; i<sizeof(cEagle)/sizeof(float); i+=4)
  {
    ed.index = i;
    myExplosionDebris.push_back(ed);
  }
}

// ----------------------------------------------------------------------------------------
void Eagle::step(unsigned long dt)
{
  if (isActive() == false)
    return;

  // ---- Lander exploded

  if (myExplosionInProgress == true)
  {
    // Move, rotate each element
    for (auto& ed: myExplosionDebris) 
    {
      ed.angle += ed.rotationSpeed*dt/1000;
      ed.pos += ed.velocity*dt/1000;
    }    
    
    // Do not process the remaining part of the step() method
    return;
  }

  // ---- Abort maneuver

  if (myAbortInProgress == true)
  {
    if (myAngle > 0)
    {
      setAngle(max(0.0f, myAngle-10));
    }
    else if (myAngle < 0)
    {
      setAngle(min(0.0f, myAngle+10));
    }
    else 
    {
      myThrust = cMaxThrust * 2; // applyThrust() not used since it does not allow for "extra" thrust
      playRocketSound();
    }

    if (myVelocity.x > 0)
    {
      myVelocity.x = max(0.0f, myVelocity.x-0.1f);
    }
    else if (myVelocity.x < 0)
    {
      myVelocity.x = min(0.0f, myVelocity.x+0.1f);
    }

    if (floatEquals(myAngle, 0) && floatEquals(myVelocity.x, 0) && myVelocity.y <= -1.1f)
    {
      stopAbort();
    }
  }
 
  // ---- Physics simulation

  myFuel -= (myThrust / cMaxThrust * cFuelBurnRate * dt / 1000.0f);

  if (myFuel <= 0)
  {
    myFuel   = 0;
    myThrust = 0;
    mySound.stop(mySounds.rocketEngine);
    mySound.stop(mySounds.fuelAlarm);
  }

  if (myFuel < cFuelLow && myFuel > 0 && mySound.isPlaying(mySounds.fuelAlarm) == false)
  {
    mySound.play(mySounds.fuelAlarm, true);
  }

  // Arcade version reduces horizontal speed over time ("air resistance" ;-) ) -> linear model applied
  //   float drag = 0.5*myVelocity.x; 
  // Quadratic version is more realistic but not required
  //   float drag = 0.5*myVelocity.x*myVelocity.x*(myVelocity.x>0 ? 1 : -1);
  
  float ax = myThrust * sinf(myAngleRad) - 0.5f*myVelocity.x;
  float ay = cGravity - myThrust * cosf(myAngleRad);

  // Divide by 10, since one unit denotes 10m, i.e. 240 "pixel" = 2400m
  myVelocity += (Vector2(ax*dt/1000, ay*dt/1000) / 10);
  myPos      += ( myVelocity / 10);
}

// ----------------------------------------------------------------------------------------
void Eagle::draw()
{
  if (isActive() == false)
    return;

  canvas().setPenColor(myColor);
  canvas().setBrushColor(Color::Black);    
  canvas().setPenWidth(1);

  if (myExplosionInProgress == true)
  {
    for (auto& ed: myExplosionDebris) 
    {
      drawLine(Vector2(cEagle[ed.index],   cEagle[ed.index+1]).rotate(ed.angle*deg2rad) * myScale + ed.pos, 
               Vector2(cEagle[ed.index+2], cEagle[ed.index+3]).rotate(ed.angle*deg2rad) * myScale + ed.pos);
    }
  }
  else 
  {
    for (size_t i=0; i<sizeof(cEagle)/sizeof(float); i+=4)
    {
      moveTo(rotateScaleTranslate(cEagle[i],   cEagle[i+1]));
      lineTo(rotateScaleTranslate(cEagle[i+2], cEagle[i+3]));
    }

    // Rocket flame
    float size = (myThrust / cMaxThrust * 125 - random(-10, 10)) / 40;
    if (myThrust > 0 && size > 0)
    {
      drawLine(rotateScaleTranslate(-0.433f, 0.818f), rotateScaleTranslate(0.433f, 0.818f));
      drawLine(rotateScaleTranslate(-0.433f, 0.818f), rotateScaleTranslate(0, 0.818f+size));
      drawLine(rotateScaleTranslate( 0.433f, 0.818f), rotateScaleTranslate(0, 0.818f+size)); 
    }

    if (myIsDebugOn)
    {
      canvas().setPenColor(Color::Yellow);
      canvas().drawRectangle(myCamera.worldToScreenX(myBBMin.x+myPos.x), myCamera.worldToScreenY(myBBMin.y+myPos.y), 
                             myCamera.worldToScreenX(myBBMax.x+myPos.x), myCamera.worldToScreenY(myBBMax.y+myPos.y));
      canvas().drawEllipse(myCamera.worldToScreenX(myPos.x), myCamera.worldToScreenY(myPos.y), 3, 3);

      canvas().setPenColor(myColor);
      canvas().selectFont(&fabgl::FONT_6x8);
      canvas().drawTextFmt(myCamera.worldToScreenX(myBBMax.x+myPos.x)+5, myCamera.worldToScreenY(myPos.y)-2, "h: %.2f", (double)myVelocity.x);
      canvas().drawTextFmt(myCamera.worldToScreenX(myBBMax.x+myPos.x)+5, myCamera.worldToScreenY(myPos.y)+10, "v: %.2f", (double)myVelocity.y);
      // canvas().drawTextFmt(myCamera.worldToScreenX(myBBMax.x+myPos.x)+5, myCamera.worldToScreenY(myPos.y)-2, "t: %3.1f", myThrust);
      // canvas().drawText(myCamera.worldToScreenX(myBBMax.x+myPos.x)+5, myCamera.worldToScreenY(myPos.y)-2, myPos.toString().c_str());
      // canvas().drawText(myCamera.worldToScreenX(myBBMax.x+myPos.x)+5, myCamera.worldToScreenY(myPos.y)+10, myCamera.worldToScreen(myPos).toString().c_str());
      // canvas().drawText(myCamera.worldToScreenX(myBBMax.x+myPos.x)+5, myCamera.worldToScreenY(myPos.y)+20, myCamera.getPos().toString().c_str());
    }
  }
}

// ----------------------------------------------------------------------------------------
void Eagle::reset(bool resetFuel)
{
  myScale    = 1;
  myPos      = Vector2(0, 0);
  myVelocity = Vector2(0, 0);
  myAngle    = myAngleRad = 0;
  myThrust   = 0;
  myAbortInProgress = false;
  myExplosionInProgress = false;

  if (resetFuel == true)
    myFuel = cStartFuel;

  computeRotateScaleConstants();
  computeBoundingBox();

  mySound.stop(mySounds.rocketEngine);
  mySound.stop(mySounds.fuelAlarm);
}

// ----------------------------------------------------------------------------------------
void Eagle::hasLanded()
{
  myThrust = 0;
  mySound.stop(mySounds.rocketEngine);
  mySound.stop(mySounds.fuelAlarm);  
}

// ----------------------------------------------------------------------------------------
void Eagle::hasCrashed()
{
  myExplosionInProgress = true;

  myThrust = 0;
  mySound.stop(mySounds.rocketEngine);
  mySound.stop(mySounds.fuelAlarm); 
  mySound.play(mySounds.explosion);

  myFuel = max(myFuel - 240, 0.0f);

  // Initialize debris position, velocity, etc.
  for (auto& ed: myExplosionDebris) 
  {
    ed.angle         = myAngle;
    ed.rotationSpeed = random(180);
    ed.pos           = myPos;
    ed.velocity      = Vector2(random(-4, 4), random(-5, -2));
  }
}

// ----------------------------------------------------------------------------------------
void Eagle::abort()
{
  myAbortInProgress = true;
  myAbortFuelLevel = myFuel;
}

// ----------------------------------------------------------------------------------------
void Eagle::stopAbort()
{
  if (myAbortInProgress == true)
  {
    myAbortInProgress = false;
    // Abort maneuver takes at least 89l of fuel
    if (myAbortFuelLevel-myFuel < 89)
      myFuel = myAbortFuelLevel-89;
    applyThrust(0);
  }
}

// ----------------------------------------------------------------------------------------
void Eagle::setColor(const RGB888& color)
{
  myColor = color;
}

// ----------------------------------------------------------------------------------------
void Eagle::applyThrust(float thrust)
{
  // In case of an active abort maneuver or explosion, no manual thrust control is accepted
  if (myAbortInProgress == true || myExplosionInProgress == true)
    return;
  
  myThrust = constrain(thrust, 0.0f, cMaxThrust);

  playRocketSound();
}

// ----------------------------------------------------------------------------------------
void Eagle::setScale(float scale)
{
  if (scale <= 0)
  {
    Serial << __PRETTY_FUNCTION__  << " -> invalid 'scale': " << scale << endl;
  }
  else 
  {
    myScale = scale;
    computeRotateScaleConstants();
    computeBoundingBox();
  }
}

// ----------------------------------------------------------------------------------------
void Eagle::setAngle(float angle)
{ 
  myAngle = constrain(angle, -90.0f, 90.0f);
  myAngleRad = deg2rad * myAngle;
  computeRotateScaleConstants();
  computeBoundingBox();
}

// ----------------------------------------------------------------------------------------
void Eagle::setFuel(float fuel)
{
  myFuel = (fuel < 0 ? 0 : fuel);
  if (myFuel > cMaxFuel)
    myFuel = cMaxFuel;
}

// ----------------------------------------------------------------------------------------
void Eagle::playRocketSound()
{
  if (myFuel > 0)
  {
    if (myThrust > 0)
    {
      // If abort is in progress, then myThrust is > Eagle::cMaxThrust -> constrain to [0, cMaxThrust]
      int newVolume = (int)mapf(min(myThrust, Eagle::cMaxThrust), 0, Eagle::cMaxThrust, 10, 80);
      if (mySound.isPlaying(mySounds.rocketEngine) == false)
      {
        mySound.setVolume(mySounds.rocketEngine, newVolume);
        mySound.play(mySounds.rocketEngine, true);
        myRocketVolume = newVolume;
      }
      else if (newVolume > myRocketVolume+10 || newVolume < myRocketVolume-10)
      {
        mySound.setVolume(mySounds.rocketEngine, newVolume);
        myRocketVolume = newVolume;
      }
    }
    else
    {
      mySound.stop(mySounds.rocketEngine);
    } 
  }
}

// ----------------------------------------------------------------------------------------
void Eagle::computeRotateScaleConstants()
{
  // Method computes the constants for a combined rotate and scale operation
  //  ┌                ┐ ┌      ┐    ┌                        ┐
  //  |  cos a  -sin a | | s  0 | =  |  s * cos a  -s * sin a |
  //  |  sin a   cos a | | 0  s |    |  s * sin a   s * cos a |
  //  └                ┘ └      ┘    └                        ┘

  myCosScale = cosf(myAngleRad) * myScale;
  mySinScale = sinf(myAngleRad) * myScale;
}

// ----------------------------------------------------------------------------------------
Vector2 Eagle::rotateScale(float x, float y)
{
  //  ┌                        ┐ ┌   ┐
  //  |  s * cos a  -s * sin a | | x |
  //  |  s * sin a   s * cos a | | y |
  //  └                        ┘ └   ┘

  return Vector2 (myCosScale * x - mySinScale * y,  
                  mySinScale * x + myCosScale * y);
}

// ----------------------------------------------------------------------------------------
Vector2 Eagle::rotateScaleTranslate(float x, float y)
{
  return Vector2(myCosScale * x - mySinScale * y + myPos.x,  
                 mySinScale * x + myCosScale * y + myPos.y);
}

// ----------------------------------------------------------------------------------------
void Eagle::computeBoundingBox()
{
  float minX, maxX, minY, maxY;

  Vector2 p(rotateScale(cEagle[0], cEagle[1]));

  minX = maxX = p.x;
  minY = maxY = p.y;

  for (size_t i=2; i<sizeof(cEagle)/sizeof(float); i+=2)
  {
    p = Vector2(rotateScale(cEagle[i], cEagle[i+1]));

    minX = min(minX, p.x);
    maxX = max(maxX, p.x);
    minY = min(minY, p.y);
    maxY = max(maxY, p.y);
  }

  // Reduce the width of the bounding box to add some tolerance
  float tolerance = (maxX-minX) * 0.15f; 

  myBBMin = Vector2(minX + tolerance, minY);
  myBBMax = Vector2(maxX - tolerance, maxY);
}

// ----------------------------------------------------------------------------------------
void Eagle::drawShape(const Vector2& screenPos, float scale, const RGB888& color)
{
  canvas().setPenColor(color);
  canvas().setPenWidth(1);

  for (size_t i=0; i<sizeof(cEagle)/sizeof(float); i+=4)
  {
    canvas().drawLine(screenPos.x + cEagle[i]  *scale, screenPos.y + cEagle[i+1]*scale,
                      screenPos.x + cEagle[i+2]*scale, screenPos.y + cEagle[i+3]*scale);
  }
}

// ----------------------------------------------------------------------------------------
void Eagle::moveTo(const Vector2& p)
{ 
  canvas().moveTo(myCamera.worldToScreenX(p.x), myCamera.worldToScreenY(p.y)); 
}

// ----------------------------------------------------------------------------------------
void Eagle::lineTo(const Vector2& p)
{ 
  canvas().lineTo(myCamera.worldToScreenX(p.x), myCamera.worldToScreenY(p.y)); 
}

// ----------------------------------------------------------------------------------------
void Eagle::drawLine(const Vector2& p1, const Vector2& p2) 
{ 
  canvas().drawLine(myCamera.worldToScreenX(p1.x), myCamera.worldToScreenY(p1.y), 
                    myCamera.worldToScreenX(p2.x), myCamera.worldToScreenY(p2.y)); 
}


