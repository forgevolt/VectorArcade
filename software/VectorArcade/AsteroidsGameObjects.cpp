#include "AsteroidsGameObjects.h"
#include "AsteroidsSounds.h"
#include <Streaming.h>

namespace
{

// ----------------------------------------------------------------------------------------
void wrapAround(Vector2& pos, int width, int height)
{
  if (pos.x < 0)
    pos.x = width;
  else if (pos.x > width)
    pos.x = 0;

  if (pos.y < 0)
    pos.y = height;
  else if (pos.y > height)
    pos.y = 0;
}

// ----------------------------------------------------------------------------------------
void wrapAroundY(float& y, int height)
{
  if (y < 0)
    y = height;
  else if (y > height)
    y = 0;
}


// ----------------------------------------------------------------------------------------
// Helpers to make use of class Vector2
void moveTo(fabgl::Canvas& canvas, const Vector2& p)
{ 
  canvas.moveTo(p.x, p.y);
}

// ----------------------------------------------------------------------------------------
void lineTo(fabgl::Canvas& canvas, const Vector2& p)
{ 
  canvas.lineTo(p.x, p.y); 
}

// ----------------------------------------------------------------------------------------
void drawLine(fabgl::Canvas& canvas, const Vector2& p1, const Vector2& p2) 
{ 
  canvas.drawLine(p1.x, p1.y, p2.x, p2.y); 
}

} // namespace


// ---- Ship ------------------------------------------------------------------------------

namespace
{

constexpr Vector2 cShipData[] = {
  {  0.000, -8.472 },
  {  4.748,  6.091 },  
  {  2.295,  3.738 },  
  { -2.295,  3.738 }, 
  { -4.748,  6.091 }, 
  {  0.000, -8.472 }  
};

constexpr Vector2 cPlumeData [] = {
   {2.295,  3.738}, 
   {0.000,  10.619},
  {-2.295,  3.738} 
};

struct ShipShape
{
  int numVertices;
  const Vector2* vertices;      // Vertices of the ship
  int numVerticesPlume;
  const Vector2* verticesPlume; // Vertices of the plume
};

constexpr ShipShape cShip = {
  sizeof(cShipData)/sizeof(Vector2),  cShipData, 
  sizeof(cPlumeData)/sizeof(Vector2), cPlumeData
};


constexpr float cShipMaxVelocity  = 150;  // max speed m/sec (actually pixels/second)
constexpr float cShipAcceleration = 5;    // acceleration m/sec^2 
constexpr float cDampeningFactor  = 0.98; // ship slows down when thruster is not active

} // namespace


// ----------------------------------------------------------------------------------------
Ship::Ship(fabgl::Canvas& canvas, SoundEngine& sound, AsteroidsSounds& sounds)
: DisplayObject(canvas),
  mySound(sound),
  mySounds(sounds)
{
  reset();
  computeRotateScaleConstants();

  // Initialize explosion debris objects
  ExplosionDebris ed;

  for (int i=0; i<cShip.numVertices-1; i++)
  {
    ed.index = i;
    myExplosionDebris.push_back(ed);
  }
}

// ----------------------------------------------------------------------------------------
void Ship::step(unsigned long dt)
{
  if (isActive() == false)
    return;

  // ---- Ship exploded

  if (myExplosionInProgress == true)
  {
    if (millis()-myStartExplosion > cTimeShipExplosionIsVisible)
    {
      setState(eNotActive);
      myExplosionInProgress = false;
      return;
    }

    // Move, rotate each element
    for (auto& ed: myExplosionDebris) 
    {
      ed.angle += ed.rotationSpeed*dt/1000;
      ed.pos += ed.velocity*dt/1000;
    }    
    
    // Do not process the remaining part of the step() method
    return;
  }

  // ---- Hyperspace: ship reappears after a delay (600ms)

  if (myHyperspaceActive == true)
  {
    if (millis() - myStartHyperspace > 600)
      myHyperspaceActive = false;
  }
  else 
  {
    // ---- Physics simulation

    if (myThrusterIsOn == true)
    {
      // Calculate change of velocity
      Vector2 acceleration(sinf(myAngleRad) * cShipAcceleration, -cosf(myAngleRad) * cShipAcceleration);
      myVelocity += acceleration;

      // Limit velocity of the ship to cShipMaxVelocity
      float v    = myVelocity.length();
      if (v > cShipMaxVelocity)
      {
        myVelocity.x = myVelocity.x / v * cShipMaxVelocity;
        myVelocity.y = myVelocity.y / v * cShipMaxVelocity;
      }
    }
    else 
    {
      // Slow down
      myVelocity = myVelocity * cDampeningFactor;
    }

    myPos += myVelocity * dt / 1000.0f;

    // ---- Wrap around

    wrapAround(myPos, canvas().getWidth(), canvas().getHeight());
  }
}

// ----------------------------------------------------------------------------------------
void Ship::draw()
{
  if (isActive() == false)
    return;

  canvas().setPenWidth(1);

  if (myExplosionInProgress == true)
  {
    // Fade out
    uint8_t c = lerpf(0, cDefaultCol.R, 
                     (cTimeShipExplosionIsVisible-min(millis()-myStartExplosion, cTimeShipExplosionIsVisible))/float(cTimeShipExplosionIsVisible));
    canvas().setPenColor(c, c, c);

    for (auto& ed: myExplosionDebris)
    {
      drawLine(canvas(),
               cShip.vertices[ed.index].rotate(ed.angle*deg2rad) * myScale + ed.pos, 
               cShip.vertices[ed.index+1].rotate(ed.angle*deg2rad) * myScale + ed.pos);
    }    
  }
  else if (myHyperspaceActive == false)
  {
    canvas().setPenColor(cDefaultCol);

    moveTo(canvas(), rotateScaleTranslate(cShip.vertices[0]));
    for (int i=1; i<cShip.numVertices; i++)
    {
      lineTo(canvas(), rotateScaleTranslate(cShip.vertices[i]));
    }

    // Show "flickering" plume if thruster is active
    if (myThrusterIsOn == true && random(0, 10) % 2)
    {
      moveTo(canvas(), rotateScaleTranslate(cShip.verticesPlume[0]));
      for (int i=1; i<cShip.numVerticesPlume; i++)
      {
        lineTo(canvas(), rotateScaleTranslate(cShip.verticesPlume[i]));
      }      
    }
  }
}

// ----------------------------------------------------------------------------------------
void Ship::drawShipShape(const Vector2& pos, float angle, float scale, const RGB888& color)
{
  canvas().setPenColor(color);
  canvas().setPenWidth(1);

  moveTo(canvas(), cShip.vertices[0].rotate(angle*deg2rad) * scale + pos);
  for (int i=1; i<cShip.numVertices; i++)
  {
    lineTo(canvas(), cShip.vertices[i].rotate(angle*deg2rad) * scale + pos);
  }
}

// ----------------------------------------------------------------------------------------
void Ship::reset()
{
  myPos                 = Vector2(canvas().getWidth()/2, canvas().getHeight()/2); 
  myVelocity            = Vector2(0, 0);
  myThrusterIsOn        = false;
  myExplosionInProgress = false;
  myHyperspaceActive    = false;
  setScale(1);
  setAngle(90);
  setState(eActive);
  mySound.stop(mySounds.rocketEngine);
}

// ----------------------------------------------------------------------------------------
void Ship::setScale(float scale)
{
  if (scale <= 0)
  {
    Serial << __PRETTY_FUNCTION__  << " -> invalid 'scale': " << scale << endl;
  }
  else 
  {
    myScale = scale;
    computeRotateScaleConstants();
  }
}

// ----------------------------------------------------------------------------------------
void Ship::setAngle(float angle)
{ 
  myAngle = fmodf(angle, 360.0f);
  myAngleRad = deg2rad * myAngle;
  computeRotateScaleConstants();
}

// ----------------------------------------------------------------------------------------
void Ship::changeAngle(float angle)
{ 
  setAngle(myAngle + angle);
}

// ----------------------------------------------------------------------------------------
void Ship::thrust(bool isOn)
{
  myThrusterIsOn = isOn;
  if (myThrusterIsOn == true)
  {
    if (mySound.isPlaying(mySounds.rocketEngine) == false)
    {
      mySound.play(mySounds.rocketEngine, true);    
    }
  }
  else 
  {
    mySound.stop(mySounds.rocketEngine);
  }
}

// ----------------------------------------------------------------------------------------
void Ship::hyperspace()
{
  // Ship already in hyperspace?
  if (myHyperspaceActive == true)
    return;

  myHyperspaceActive = true;
  myStartHyperspace = millis();
  myPos = Vector2(random(20, canvas().getWidth()-20), random(20, canvas().getHeight()-20));
  myVelocity = Vector2(0, 0);
}

// ----------------------------------------------------------------------------------------
bool Ship::detectCollision(const Asteroid& a) const
{
  bool collision = false;

  for (int i=0; i<cShip.numVertices; i++)
  {
    collision |= a.detectCollision(rotateScaleTranslate(cShip.vertices[i]));
  }

  return collision;
}

// ----------------------------------------------------------------------------------------
bool Ship::detectCollision(const Saucer& s) const
{
  bool collision = false;

  for (int i=0; i<cShip.numVertices; i++)
  {
    collision |= s.detectCollision(rotateScaleTranslate(cShip.vertices[i]));
  }

  return collision;
}

// ----------------------------------------------------------------------------------------
bool Ship::detectCollision(const Shot& s) const
{
  Vector2 p = s.getPos();

  // Based on https://www.geeksforgeeks.org/point-in-polygon-in-cpp/ (crossing number algorithm)
  bool inside = false;
  Vector2 p1, p2;

  // Iterate through each edge of the polygon
  for (int i = 0, j = cShip.numVertices - 1; i < cShip.numVertices; j = i++) 
  {
    p1 = rotateScaleTranslate(cShip.vertices[j]);
    p2 = rotateScaleTranslate(cShip.vertices[i]);

    // Check if the point is between the y-coordinates
    // of p1 and p2 
    if (((p1.y > p.y) != (p2.y > p.y)) &&
    // && calculate the x-coordinate where the ray from the
    // point intersects the edge and check if the point lies to the left of the
    // intersection
        (p.x < (p2.x - p1.x) * (p.y - p1.y) / (p2.y - p1.y) + p1.x))
    {
      inside = !inside;
    }
  }

  // Return true if point is inside the polygon, false otherwise
  return inside;  
}

// ----------------------------------------------------------------------------------------
void Ship::gotHit()
{
  mySound.stop(mySounds.rocketEngine);
  mySound.play(mySounds.explosion);

  myExplosionInProgress = true;
  myStartExplosion      = millis();
  myThrusterIsOn        = false;

  // Initialize debris position, velocity, etc.
  for (auto& ed: myExplosionDebris) 
  {
    ed.angle         = myAngle;
    ed.rotationSpeed = random(90);
    ed.pos           = myPos;
    ed.velocity      = Vector2(random(-15, 15), random(-15, 15));
  }  
}

// ----------------------------------------------------------------------------------------
void Ship::computeRotateScaleConstants()
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
Vector2 Ship::rotateScaleTranslate(const Vector2& p) const
{
  return Vector2 (myCosScale * p.x - mySinScale * p.y + myPos.x,  
                  mySinScale * p.x + myCosScale * p.y + myPos.y);
}


// ---- Shot ------------------------------------------------------------------------------

namespace
{

constexpr float cShotVelocity       = 200;  // max speed m/sec (actually pixels/second)
constexpr float cSaucerShotVelocity = cShotVelocity / 3;

} // namespace

// ----------------------------------------------------------------------------------------
Shot::Shot(fabgl::Canvas& canvas, bool isSaucerShot)
: DisplayObject(canvas),
  myIsSaucerShot(isSaucerShot)
{
  reset();
}

// ----------------------------------------------------------------------------------------
void Shot::step(unsigned long dt)
{
  if (isActive() == false)
    return;

  myPos += myVelocity * dt / 1000;
  myShotActiveTime += dt;

  // Shot moves for approx. cShipShotRange pixels (ship) and cSaucerShotRange pixels (saucer) until it disappears
  if ((myIsSaucerShot == false && myShotActiveTime >= (Layout::cShipShotRange / cShotVelocity * 1000)) ||
      (myIsSaucerShot == true  && myShotActiveTime >= (Layout::cSaucerShotRange / cSaucerShotVelocity * 1000)))
  {
    setState(eNotActive);
  }

  // ---- Wrap around

  if (myIsSaucerShot == false)
    wrapAround(myPos, canvas().getWidth(), canvas().getHeight());
}

// ----------------------------------------------------------------------------------------
void Shot::draw()
{
  if (isActive() == false)
    return;

  if (myIsSaucerShot == false)
  {
    canvas().setPenColor(Color::BrightWhite);
    canvas().setPixel(myPos.x, myPos.y);
  }
  else 
  {
    canvas().setBrushColor(cDefaultCol);
    canvas().fillEllipse(myPos.x, myPos.y, 3, 4);
  }
}

// ----------------------------------------------------------------------------------------
void Shot::reset()
{
  setState(eNotActive);
}

// ----------------------------------------------------------------------------------------
void Shot::fire(const Vector2& pos, const Vector2& direction)
{
  myPos = pos;
  myVelocity = direction.normalize() * (myIsSaucerShot == false ? cShotVelocity : cSaucerShotVelocity);
  myShotActiveTime = 0;
  setState(eActive);
}

// ----------------------------------------------------------------------------------------
bool Shot::detectCollision(const Asteroid& a) const
{
  return a.detectCollision(myPos);
}

// ----------------------------------------------------------------------------------------
bool Shot::detectCollision(const Saucer& s) const
{
  return s.detectCollision(myPos);
}

// ---- Asteroid --------------------------------------------------------------------------

namespace
{

constexpr Vector2 cAsteroidData1[] = {
  {  -7.165, -16.111 },
  {   0.650, -11.547 }, 
  {   8.521, -16.575 },
  {  16.501,  -7.502 },
  {   8.985,  -3.212 },
  {  16.055,   4.019 },
  {   8.412,  15.808 },
  {  -3.613,  11.791 },
  {  -7.275,  16.300 },
  { -15.910,   7.774 },
  { -11.183,   0.149 },
  { -15.692,  -7.830 },
  {  -7.165, -16.111 }
};

constexpr Vector2 cAsteroidData2[] = {
  {  -7.411, -16.081 },
  {   4.165, -16.121 },
  {  16.291,  -7.723 },
  {  15.977,  -3.662 },
  {   5.107,  -0.326 },
  {  16.232,   7.797 },
  {   8.855,  15.527 },
  {   4.244,  11.956 },
  {  -7.666,  15.763 },
  { -15.279,   4.246 }, 
  { -15.141,  -7.998 },
  {  -3.604,  -7.939 },
  {  -7.411, -16.081 }
};

constexpr Vector2 cAsteroidData3[] = {
  {  -5.121, -15.398 },
  {   6.442, -15.492 },
  {  14.973,  -3.466 },
  {  15.076,   4.748 },
  {   6.760,  16.112 },
  {  -0.524,  16.215 },
  {  -0.575,   4.283 },
  {  -8.615,  16.270 },
  { -17.267,   4.559 },
  {  -8.924,   0.773 },
  { -17.002,  -3.740 },
  {  -5.121, -15.398 }
};

struct AsteroidShape 
{
  int numVertices;
  const Vector2* vertices;        // Vertices of the asteroid
  float minX, maxX, minY, maxY;   // Its bounding box
};

constexpr AsteroidShape cAsteroids[3] = {
  { sizeof(cAsteroidData1)/sizeof(Vector2), cAsteroidData1, -15.910, 16.501, -16.575, 16.300 },
  { sizeof(cAsteroidData2)/sizeof(Vector2), cAsteroidData2, -15.279, 16.291, -16.121, 15.763 },
  { sizeof(cAsteroidData3)/sizeof(Vector2), cAsteroidData3, -17.267, 15.076, -15.492, 16.270 }
};

constexpr unsigned long cTimeAsteroidExplosionIsVisible = 1500;

} // namespace

// ----------------------------------------------------------------------------------------
Asteroid::Asteroid(fabgl::Canvas& canvas, SoundEngine& sound, AsteroidsSounds& sounds)
: DisplayObject(canvas),
  mySound(sound),
  mySounds(sounds),
  myExplosion(20)
{
  reset();
}

// ----------------------------------------------------------------------------------------
void Asteroid::step(unsigned long dt)
{
  if (isActive() == false)
    return;

  if (myExplosionInProgress == true)
  {
    if (millis() - myStartExplosion < cTimeAsteroidExplosionIsVisible)
    {
      for (size_t i=0; i<myExplosion.size(); i+=2)
      {
        myExplosion[i] =  myExplosion[i] + myExplosion[i+1] * dt / 1000;
      }
    }
    else 
    {
      myExplosionInProgress = false;
      setState(eNotActive);
    }
  }
  else 
  {
    myPos += myVelocity * dt / 1000;
    wrapAround(myPos, canvas().getWidth(), canvas().getHeight());
  }
}

// ----------------------------------------------------------------------------------------
void Asteroid::draw()
{
  if (isActive() == false)
    return;

  if (myExplosionInProgress == true)
  {
    // Fade out
    uint8_t c = lerpf(0, cDefaultCol.R, 
                     (cTimeAsteroidExplosionIsVisible-min(millis()-myStartExplosion, cTimeAsteroidExplosionIsVisible))/float(cTimeAsteroidExplosionIsVisible));
    canvas().setPenColor(c, c, c);

    for (size_t i=0; i<myExplosion.size(); i+=2)
      canvas().setPixel(myExplosion[i].x, myExplosion[i].y);
  }
  else 
  {
    canvas().setPenColor(cDefaultCol);
    canvas().setPenWidth(1);

    moveTo(canvas(), cAsteroids[myShape].vertices[0]*myScale + myPos);
    for (int i=1; i<cAsteroids[myShape].numVertices; i++)
    {
      lineTo(canvas(), cAsteroids[myShape].vertices[i]*myScale + myPos);
    }
  }
}

// ----------------------------------------------------------------------------------------
void Asteroid::reset()
{
  myExplosionInProgress = false;
  setState(eNotActive);
  newShape(eLarge);
}

// ----------------------------------------------------------------------------------------
void Asteroid::newShape(ESize size)
{
  switch (size)
  {
    case eSmall:
      mySize = eSmall;
      myScale = 0.4;
      break;
    case eMedium:
      mySize = eMedium;
      myScale = 0.6;
      break;
    case eLarge:
    default:
      mySize = eLarge;
      myScale = 1.0;
      break;
  }  
  myShape = random(3);
}

// ----------------------------------------------------------------------------------------
bool Asteroid::detectCollision(const Vector2& checkPoint) const
{
  if (isActive() == false || myExplosionInProgress == true)
    return false;

  // Translate checkPoint so that we check with the original positions of the vertices
  Vector2 p = checkPoint-myPos;

  // Bounding box check
  if (p.x < cAsteroids[myShape].minX * myScale || 
      p.x > cAsteroids[myShape].maxX * myScale || 
      p.y < cAsteroids[myShape].minY * myScale || 
      p.y > cAsteroids[myShape].maxY * myScale) 
  {
    return false;
  }

  // Based on https://www.geeksforgeeks.org/point-in-polygon-in-cpp/ (crossing number algorithm)
  bool inside = false;
  Vector2 p1, p2;

  // Iterate through each edge of the polygon
  for (int i = 0, j = cAsteroids[myShape].numVertices - 1; i < cAsteroids[myShape].numVertices; j = i++) 
  {
    p1 = cAsteroids[myShape].vertices[j]*myScale;
    p2 = cAsteroids[myShape].vertices[i]*myScale;

    // Check if the point is between the y-coordinates
    // of p1 and p2 
    if (((p1.y > p.y) != (p2.y > p.y)) &&
    // && calculate the x-coordinate where the ray from the
    // point intersects the edge and check if the point lies to the left of the
    // intersection
        (p.x < (p2.x - p1.x) * (p.y - p1.y) / (p2.y - p1.y) + p1.x))
    {
      inside = !inside;
    }
  }

  // Return true if point is inside the polygon, false otherwise
  return inside;
}

// ----------------------------------------------------------------------------------------
void Asteroid::gotHit()
{
  mySound.play(mySounds.smallExplosion);

  myExplosionInProgress = true;
  myStartExplosion = millis();

  for (size_t i=0; i<myExplosion.size(); i+=2)
  {
    myExplosion[i] = myPos;
    myExplosion[i+1] = Vector2(random(-20, 20), random(-20, 20));
  }
}


// ---- Saucer ----------------------------------------------------------------------------

namespace
{

constexpr Vector2 cSaucerData[] = {
  { -1.969, -6.783 },
  {  1.969, -6.783 }, 
  {  3.950, -3.071 }, 
  {  9.407,  0.630 },
  {  3.950,  4.357 }, 
  { -3.950,  4.357 }, 
  { -9.407,  0.630 }, 
  { -3.950, -3.071 }, 
  { -1.969, -6.783 }, 

  // Horizontal lines
  { -3.950, -3.071 }, 
  {  3.950, -3.071 }, 
  {  9.407,  0.630 }, 
  { -9.407,  0.630 } 
};

struct SaucerShape
{
  int numVertices;
  const Vector2* vertices;        // Vertices of the saucer
  float minX, maxX, minY, maxY;   // Its bounding box
};

constexpr SaucerShape cSaucer = {
  sizeof(cSaucerData)/sizeof(Vector2), cSaucerData, 
  -9.407*0.9, 9.407*0.9, -6.783*0.9, 4.357*0.9 // shrink the bounding box, since detectCollision is based on the bounding box only 
};

constexpr int cMaxSaucerShots = 5;
constexpr long cShotCoolDownTime = 1200;
constexpr unsigned long cTimeSaucerExplosionIsVisible = 1500;

} // namespace

// ----------------------------------------------------------------------------------------
Saucer::Saucer(fabgl::Canvas& canvas, SoundEngine& sound, AsteroidsSounds& sounds, const Ship& ship)
: DisplayObject(canvas),
  mySound(sound),
  mySounds(sounds),
  myShip(ship),
  myExplosion(20)
{
  for (int i=0; i<cMaxSaucerShots; i++)
  {
    myShots.push_back(new Shot(canvas, true));
  }

  reset();
}

// ----------------------------------------------------------------------------------------
Saucer::~Saucer()
{
  for (auto& s : myShots) delete s;
}

// ----------------------------------------------------------------------------------------
void Saucer::step(unsigned long dt)
{
  if (isActive() == false)
    return;

  for (auto& s : myShots) s->step(dt);

  if (myExplosionInProgress == true)
  {
    if (millis() - myStartExplosion < cTimeAsteroidExplosionIsVisible)
    {
      for (size_t i=0; i<myExplosion.size(); i+=2)
      {
        myExplosion[i] =  myExplosion[i] + myExplosion[i+1] * dt / 1000;
      }
    }
    else 
    {
      // Stay active until all shots are in state eNotActive
      bool shotsActive = false;
      for (auto& shot : myShots) shotsActive |= shot->isActive();

      if (shotsActive == false)
      {  
        myExplosionInProgress = false;
        setState(eNotActive);
      }
    }
  }
  else 
  {
    myPos += myVelocity[int(myPos.x / cSegmentWidth)] * dt / 1000;
    wrapAroundY(myPos.y, canvas().getHeight());
 
    if (myPos.x < 0 || myPos.x > canvas().getWidth())
    {
      // Stay active until all shots are in state eNotActive
      bool shotsActive = false;
      for (auto& shot : myShots) shotsActive |= shot->isActive();

      if (shotsActive == false)
        setState(eNotActive);  
    }
    else 
    {
      myShotCoolDownTime -= dt;
      if (myShotCoolDownTime <= 0)
      {
        for (auto& s : myShots)
        {
          if (s->isActive() == false)
          {
            mySound.play(mySounds.pewSaucer);
            
            Vector2 direction;

            // Big saucer: fire randomly
            if (mySize == eBig)
            {
              int angle = random(360);
              direction = Vector2(sinf(angle*deg2rad), -cosf(angle*deg2rad));
            }
            // Small saucer: aim
            else 
            {
              // Shoot in the direction of the ship
              direction = (myShip.getPos() - myPos).rotate((1-myAccuracy)*random(-25, 25)*deg2rad).normalize();
            }

            // Add 10 pixels to the starting position so that the shot starts outside the saucer
            s->fire(myPos+direction*10, direction);           
            myShotCoolDownTime = cShotCoolDownTime;
            break;
          }
        }
      }
    }
  }
}

// ----------------------------------------------------------------------------------------
void Saucer::draw()
{
  if (isActive() == false)
    return;

  for (auto& s : myShots) s->draw();

  if (myExplosionInProgress == true)
  {
    // Fade out
    uint8_t c = lerpf(0, cDefaultCol.R, 
                     (cTimeSaucerExplosionIsVisible-min(millis()-myStartExplosion, cTimeSaucerExplosionIsVisible))/float(cTimeSaucerExplosionIsVisible));
    canvas().setPenColor(c, c, c);

    for (size_t i=0; i<myExplosion.size(); i+=2)
      canvas().setPixel(myExplosion[i].x, myExplosion[i].y);
  }
  else 
  {
    canvas().setPenColor(cDefaultCol);
    canvas().setPenWidth(1);

    moveTo(canvas(), cSaucer.vertices[0]*myScale + myPos);
    for (int i=1; i<cSaucer.numVertices; i++)
    {
      lineTo(canvas(), cSaucer.vertices[i]*myScale + myPos);
    }
  }
}

// ----------------------------------------------------------------------------------------
void Saucer::reset()
{
  myExplosionInProgress = false;
  setState(eNotActive);
}

// ----------------------------------------------------------------------------------------
void Saucer::start(ESize size, float accuracy)
{
  myAccuracy = clampf(accuracy, 0.0f, 1.0f);

  float velocityX;

  if (random(2))
  {
    myPos.x = 0;
    myPos.y = random(0, myCanvas.getHeight());
    velocityX = 1; 
  }
  else
  {
    myPos.x = canvas().getWidth();
    myPos.y = random(0, myCanvas.getHeight());
    velocityX = -1; 
  }

  for (int i=0; i<cNumSegments; i++)
  {
    myVelocity[i].x = velocityX;
    myVelocity[i].y = random(-1, 1);
    myVelocity[i] = myVelocity[i].normalize() * 50;
  }

  setState(eActive);
  mySound.play(mySounds.saucerAlert);

  mySize = size;
  myScale = (mySize == eBig ? 1.3 : 1.0);
  myShotCoolDownTime = 200;
}

// ----------------------------------------------------------------------------------------
bool Saucer::detectCollision(const Vector2& checkPoint) const
{
  if (isActive() == false || myExplosionInProgress == true)
    return false;

  // Translate checkPoint so that we check with the original positions of the vertices
  Vector2 p = checkPoint-myPos;

  // Bounding box check
  if (p.x < cSaucer.minX * myScale || 
      p.x > cSaucer.maxX * myScale || 
      p.y < cSaucer.minY * myScale || 
      p.y > cSaucer.maxY * myScale) 
  {
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------------------------
bool Saucer::detectCollisionWithShot(const Ship& ship) const
{
  for (auto& shot : myShots)
  {
    if (shot->isActive() == true)
    {
      if (ship.detectCollision(*shot) == true)
      {
        shot->setState(eNotActive);
        return true;
      }
    }
  }

  return false;  
}

// ----------------------------------------------------------------------------------------
void Saucer::gotHit()
{
  mySound.play(mySounds.smallExplosion);

  myExplosionInProgress = true;
  myStartExplosion = millis();

  for (size_t i=0; i<myExplosion.size(); i+=2)
  {
    myExplosion[i] = myPos;
    myExplosion[i+1] = Vector2(random(-20, 20), random(-20, 20));
  }
}
