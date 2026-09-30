#include "Asteroids.h"
#include "Board.h"
#include <Preferences.h>
#include <Streaming.h>
#include "AsteroidsSplash.h"

#include "AsteroidsSounds.h"
#if !SOUNDS_FROM_LITTLEFS()
  #include "sounds/RocketEngine.h"
  #include "sounds/Explosion.h"
  #include "sounds/SmallExplosion.h"
  #include "sounds/Pew.h"
  #include "sounds/PewSaucer.h"
  #include "sounds/SaucerAlert.h"
#endif

// ---- AsteroidsSounds -------------------------------------------------------------------

#if SOUNDS_FROM_LITTLEFS()
// ----------------------------------------------------------------------------------------
AsteroidsSounds::AsteroidsSounds(SoundEngine& sound)
{
  loadSoundClip(sound, rocketEngine,   "/RocketEngine.wav");
  loadSoundClip(sound, explosion,      "/Explosion.wav");
  loadSoundClip(sound, smallExplosion, "/SmallExplosion.wav");
  loadSoundClip(sound, pew,            "/Pew.wav");
  loadSoundClip(sound, pewSaucer,      "/PewSaucer.wav");
  loadSoundClip(sound, saucerAlert,    "/SaucerAlert.wav");
  setVolumes();
}
#else
// ----------------------------------------------------------------------------------------
AsteroidsSounds::AsteroidsSounds(SoundEngine&)
: rocketEngine(cSoundRocketEngineWAV),
  explosion(cSoundExplosionWAV),
  smallExplosion(cSoundSmallExplosionWAV),
  pew(cSoundPewWAV),
  pewSaucer(cSoundPewSaucerWAV),
  saucerAlert(cSoundSaucerAlertWAV)
{
  setVolumes();
}
#endif

// ----------------------------------------------------------------------------------------
void AsteroidsSounds::setVolumes()
{
  rocketEngine.setVolume(50);
  explosion.setVolume(30);
  smallExplosion.setVolume(20);
  pew.setVolume(70);
  pewSaucer.setVolume(10);
  saucerAlert.setVolume(70);
}


namespace
{

// ----------------------------------------------------------------------------------------
// Collision detectors
bool detectCollision(const Shot& s, const Asteroid& a)
{
  return s.detectCollision(a);
}

bool detectCollision(const Shot& s, const Saucer& saucer)
{
  return s.detectCollision(saucer);
}

bool detectCollision(const Ship& s, const Asteroid& a)
{
  return s.detectCollision(a);
}

bool detectCollision(const Ship& s, const Saucer& saucer)
{
  return s.detectCollision(saucer);
}

} // namespace


// ---- Asteroids -------------------------------------------------------------------------

namespace
{

constexpr int cMaxLargeAsteroids = 10;
constexpr int cMaxShots = 10;
constexpr unsigned long cWaitTime = 5000; // Wait 5sec after ship got hit
constexpr float cMaxRotation  = 10.0f;    // Ship rotation in degrees per frame at full joystick deflection
constexpr float cRotationExpo = 0.7f;     // Rotation curve: 0 = linear, 1 = cubic (finer control around the center)

} // namespace

// ----------------------------------------------------------------------------------------
Asteroids::Asteroids(Menu& menu)
: MenuItem(menu, "Asteroids"),
  mySounds(menu.sound()),
  myShip(canvas(), menu.sound(), mySounds),
  mySaucer(canvas(), menu.sound(), mySounds, myShip)
{
  for (int i=0; i<cMaxShots; i++)
  {
    myShots.push_back(new Shot(canvas()));
  }
}

// ----------------------------------------------------------------------------------------
Asteroids::~Asteroids()
{
  for (auto& s : myShots)     delete s;
  for (auto& a : myAsteroids) delete a;
}

// ----------------------------------------------------------------------------------------
void Asteroids::step(unsigned long dt)
{
  if (isActive() == false)
    return;

  // ---- Query input events
  myInput = myMenu.input().getState();

  // Button 'Sel' aborts the game -> back to menu
  if (myInput.sel == true)
  {
    sound().stopAll();
    sound().play(sound().click());
    setState(eNotActive); // End the game
    return;
  }

  switch (myState)
  {
    case eSplashScreen:
      stepSplashScreen(dt);
      break;

    case eRunning:
      {
        int oldScore = myScore / 10000;
        stepRunning(dt);
        // New ship every 10'000 points
        if (myScore / 10000 > oldScore)
        {
          myNumLives++;
          sound().play(sound().signal());
        }
      }
      break;

    case eLevelCleared:
      stepLevelCleared(dt);
      break;

    case eGotHit:
      stepGotHit(dt);
      break;

    case eGameOver:
      stepGameOver(dt);
      break;

    default:
      // Should never happen -> end the game
      setState(eNotActive);
      break;
  }
}

// ----------------------------------------------------------------------------------------
void Asteroids::draw()
{
  if (isActive() == false)
  {
    MenuItem::draw();
    myShip.drawShipShape(Vector2(37, myPosY), 45, 1.3, myIsSelected ? Color::Black : cDefaultCol); 
  }
  else 
  {
    // Clear screen
    canvas().setBrushColor(Color::Black);
    canvas().clear();

    switch (myState)
    {
      case eSplashScreen:
        drawSplashScreen();
        break;

      case eRunning:
        drawRunning();
        break;

      case eLevelCleared:
        drawLevelCleared();
        break;

      case eGotHit:
        drawGotHit();
        break;

      case eGameOver:
        drawGameOver();
        break;

      default:
        // Should never happen -> end the game
        setState(eNotActive);
        break;
    }
  } 
}

// ----------------------------------------------------------------------------------------
void Asteroids::start()
{
  sound().setVolume(sound().signal(), 30);

  prepareSplashScreen();

  // Read high score from NVS
  Preferences p;
  p.begin("Asteroids");
  myHighScore = p.getInt("highScore", 0);
  p.end();

  setState(eActive);
}

// ----------------------------------------------------------------------------------------
void Asteroids::prepareSplashScreen()
{
  myState = eSplashScreen;
  myShowInstructions = false;
  myStartWaitTime = millis();
}

// ----------------------------------------------------------------------------------------
void Asteroids::prepareAttempt(bool newGame)
{
  if (newGame == true)
  {
    myNumLives  = 3;
    myLevel     = 1;
    myScore     = 0;
    myShip.reset();
  }
  mySaucerAccuracy = 0;
    
  // Reset game objects
  for (auto& a : myAsteroids) a->reset(); 
  for (auto& s : myShots)     s->reset();
  mySaucer.reset();
  myTimeSaucerAppeared = millis();

  int numAsteroids = min(3 + myLevel, cMaxLargeAsteroids);
  myNumAsteroidsToShoot = numAsteroids * 7;
  myNumAsteroidsHit = 0;
  long x, y;
  Asteroid* ast;

  for (int i=0; i<numAsteroids; i++)
  {
    switch (random(4))
    {
      case 0: // top
        x = random(canvas().getWidth());
        y = random(30);
        break;
      case 1: // bottom
        x = random(canvas().getWidth());
        y = random(canvas().getHeight()-30, canvas().getHeight());
        break;
      case 2: // left
        x = random(30);
        y = random(canvas().getHeight());
        break;
      default: // 3: right
        x = random(canvas().getWidth()-30, canvas().getWidth());
        y = random(canvas().getHeight());
        break;
    }
    
    ast = getNotActiveAsteroid();
    ast->setPos(x, y);
    ast->setVelocity(random(2) ? random(10, 30+3*myLevel) : -random(10, 30+3*myLevel), 
                     random(2) ? random(10, 30+3*myLevel) : -random(10, 30+3*myLevel));
    ast->setState(eActive);
  }
}

// ----------------------------------------------------------------------------------------
Asteroid* Asteroids::getNotActiveAsteroid()
{
  for (auto& a : myAsteroids) 
  {
    if (a->isActive() == false)
      return a;
  }

  // No unused Asteroid object -> create new one
  Asteroid* a = new Asteroid(canvas(), sound(), mySounds);
  myAsteroids.push_back(a);

  return a;
}

// ----------------------------------------------------------------------------------------
void Asteroids::splitAsteroid(Asteroid& astHit)
{
  astHit.gotHit();

  myNumAsteroidsToShoot--;
  myNumAsteroidsHit++;

  // Small asteroids disappear
  if (astHit.getSize() == Asteroid::eSmall)
  {
    myScore += 100;
    return;
  }

  // Split in two asteroids
  Vector2 v1 = astHit.getVelocity().rotate(random(10, 90)*deg2rad)*random(7, 15)/10.0f;
  Vector2 v2 = astHit.getVelocity().rotate(random(-90, -10)*deg2rad)*random(7, 15)/10.0f;
  Asteroid::ESize splitSize = Asteroid::eSmall;
  
  switch (astHit.getSize())
  {
    case Asteroid::eLarge:
      myScore += 20;
      splitSize = Asteroid::eMedium;
      break;
    case Asteroid::eMedium:
      myScore += 50;
      splitSize = Asteroid::eSmall;
      v1 *= 1.2;
      v2 *= 1.2;
      break;
    case Asteroid::eSmall:
      break;    // not reached - small asteroids return early above
  }

  Asteroid* ast = getNotActiveAsteroid();

  ast->newShape(splitSize); 
  ast->setState(eActive);
  ast->setPos(astHit.getPos());
  ast->setVelocity(v1);

  ast = getNotActiveAsteroid();

  ast->newShape(splitSize);
  ast->setState(eActive);
  ast->setPos(astHit.getPos());
  ast->setVelocity(v2);
}

// ----------------------------------------------------------------------------------------
bool Asteroids::isAreaSafe(const Vector2& pos)
{
  if (mySaucer.isActive() == true)
    return false;

  for (auto& a : myAsteroids)
  {
    if (a->isActive() == true && pos.distance(a->getPos()) < 50)
      return false;
  }

  return true;
}

// ----------------------------------------------------------------------------------------
void Asteroids::stepSplashScreen(unsigned long dt)
{
  // Check start buttons
  if (myInput.start == true)
  {
    sound().play(sound().click());
    prepareAttempt(true);
    myState = eRunning;
    return;
  }

  // Toggle between splash screen bitmap and instructions graphics
  if (millis() - myStartWaitTime > 5000)
  {
    myShowInstructions = !myShowInstructions;
    myStartWaitTime = millis();
  }
}

// ----------------------------------------------------------------------------------------
void Asteroids::stepRunning(unsigned long dt)
{
  // ---- Fire

  if (myInput.a == true && myShip.isInHyperspace() == false)
  {
    sound().play(mySounds.pew);

    for (auto& s : myShots)
    {
      if (s->isActive() == false)
      {
        // Direction is based on the ship orientation in space (i.e. its rotation angle)
        Vector2 direction(sinf(myShip.getAngle()*deg2rad), -cosf(myShip.getAngle()*deg2rad));
        
        // Add 10 pixels to the starting position so that the shot starts from the front of the ship
        s->fire(myShip.getPos()+direction*10, direction); 
        break;
      }
    }
  }

  // ---- Hit an asteroid or the saucer?

  for (auto& s : myShots)
  {
    if (s->isActive() == true)
    {
      if (mySaucer.isActive() == true && detectCollision(*s, mySaucer) == true)
      {
        s->setState(eNotActive);
        mySaucer.gotHit();
        myScore += (mySaucer.getSize() == Saucer::eBig ? 200 : 1000);
        continue; // Next shot
      }
      for (auto& a : myAsteroids)
      {
         // Shot has hit an asteroid?
         if (a->isActive() && detectCollision(*s, *a) == true)
         {
           s->setState(eNotActive);
           splitAsteroid(*a);
           break; // Next shot
         }
      }
    }
  }
  
  // ---- Ship hit by an asteroid, a saucer or a shot from a saucer?

  if (myShip.isInHyperspace() == false)
  {
    bool shipGotHit = false;

    // Hit by an asteroid?
    for (auto& a : myAsteroids)
    {
      if (a->isActive() && detectCollision(myShip, *a) == true)
      {
        myShip.gotHit();
        splitAsteroid(*a);
        shipGotHit = true;
        break;
      }
    }

    // Hit by the saucer?
    if (mySaucer.isActive() && detectCollision(myShip, mySaucer) == true)
    {
      myShip.gotHit();
      mySaucer.gotHit();
      myScore += (mySaucer.getSize() == Saucer::eBig ? 200 : 1000);
      shipGotHit = true;
    }

    // Hit by a shot from the saucer?
    if (mySaucer.isActive() && mySaucer.detectCollisionWithShot(myShip) == true)
    {
      myShip.gotHit();
      shipGotHit = true;
    }

    if (shipGotHit == true)
    {
      myStartWaitTime = millis();
      myNumLives--;
      myState = (myNumLives <= 0 ? eGameOver : eGotHit);
      
      if (myNumLives <= 0 && myScore > myHighScore)
      {
        myHighScore = myScore;

        // Store new high score in NVS
        Preferences p;
        p.begin("Asteroids");
        p.putInt("highScore",  myHighScore);
        p.end();
      }
          
      return;
    }
  }

  // ---- Activate hyperspace (Aux button, Start on boards without Aux)

  if ((BOARD_HAS_AUX_BUTTON() ? myInput.aux : myInput.start) == true)
  {
    myShip.hyperspace();
  }

  // ---- Rotate ship

  // Expo curve: blend of linear and cubic response for fine control around the center
  float n = constrain(abs(myInput.x) / (float)Joystick2Axis::cMAX, 0.0f, 1.0f);
  float angle = cMaxRotation * ((1.0f - cRotationExpo) * n + cRotationExpo * n * n * n);
  if (myInput.x < 0) angle *= -1;
  myShip.changeAngle(angle);

  // Linear mapping
  // myShip.changeAngle(mapf(myInput.x, Joystick2Axis::cMIN, Joystick2Axis::cMAX, -10, 10));

  // ---- Apply Thrust

  myShip.thrust(myInput.bIsPressed);

  // ---- Start saucer
  
  if (mySaucer.isActive() == false &&               // saucer not active yet
      myNumAsteroidsToShoot < myNumAsteroidsHit &&  // more than half of the asteroids are destroyed
      myNumAsteroidsToShoot > 2 &&                  // at least 2 asteroids are left
      millis() - myTimeSaucerAppeared > 13000)      // minimum wait time since last saucer has appeared
  {
    mySaucerAccuracy += 0.1f;
    myTimeSaucerAppeared = millis();
    if (myScore > 40000 || random(2)) // After reaching a score of 40'000, only the small saucer appears
      mySaucer.start(Saucer::eSmall, mySaucerAccuracy);
    else
      mySaucer.start(Saucer::eBig);  
  }

  // ---- Process Game Objects

  myShip.step(dt);
  for (auto& s : myShots)     s->step(dt);
  for (auto& a : myAsteroids) a->step(dt);
  mySaucer.step(dt);

  // Level cleared?
  bool foundAnAsteroid = false;
  for (auto& a : myAsteroids)
    foundAnAsteroid |= a->isActive();

  if (foundAnAsteroid == false && mySaucer.isActive() == false)
  {
    myLevel++;
    myStartWaitTime = millis();
    myState = eLevelCleared;
  }  
}


// ----------------------------------------------------------------------------------------
void Asteroids::stepLevelCleared(unsigned long dt)
{
  // Wait for 3sec until new level starts
  if (millis() - myStartWaitTime > 3000)
  {
    prepareAttempt(false);
    myState = eRunning;
  }

  // ---- Rotating the ship and applying thrust is possible while waiting for the new level

  myShip.changeAngle(mapf(myInput.x, Joystick2Axis::cMIN, Joystick2Axis::cMAX, -10, 10));
  myShip.thrust(myInput.bIsPressed);

  for (auto& s : myShots) s->step(dt);
  myShip.step(dt);
}

// ----------------------------------------------------------------------------------------
void Asteroids::stepGotHit(unsigned long dt)
{
  // Wait until the ship explosion has faded
  if (millis() - myStartWaitTime > Ship::cTimeShipExplosionIsVisible)
  {
    // Ensure that position of new ship is not covered by an asteroid, then we switch to state eRunning. 
    // Otherwise: we wait until area is safe
    if (isAreaSafe(Vector2(canvas().getWidth()/2, canvas().getHeight()/2)) == true)
    {
      myShip.reset();
      myState = eRunning;
    }
  }

  // ---- Hit an asteroid?

  for (auto& s : myShots)
  {
    if (s->isActive() == true)
    {
      for (auto& a : myAsteroids)
      {
         // Shot has hit an asteroid?
         if (a->isActive() && detectCollision(*s, *a) == true)
         {
           s->setState(eNotActive);
           splitAsteroid(*a);
           break; // Next shot
         }
      }
    }
  }

  // ---- Process Game Objects

  for (auto& s : myShots)     s->step(dt);
  for (auto& a : myAsteroids) a->step(dt);
  mySaucer.step(dt);
  myShip.step(dt);
}

// ----------------------------------------------------------------------------------------
void Asteroids::stepGameOver(unsigned long dt)
{
  // Any key or 20sec -> switch to splash screen
  if ((myInput.a == true || myInput.b == true || myInput.aux || myInput.start || millis() - myStartWaitTime > 20000) && (millis() - myStartWaitTime > cWaitTime))
  {
    prepareSplashScreen();
  }

  // Process remaining asteroids
  for (auto& a : myAsteroids) a->step(dt);
  mySaucer.step(dt);
  myShip.step(dt);
}

// ----------------------------------------------------------------------------------------
void Asteroids::drawSplashScreen()
{
  if (myShowInstructions == true)
  {
    canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
    canvas().setPenColor(cDefaultCol);
    canvas().selectFont(&fabgl::FONT_std_24);

    drawCenteredText(10, myMenuName);
    canvas().drawLine(0, 40, canvas().getWidth(), 40);
    drawCenteredText(55, "Press 'Start' to play");

    drawInstructions("Quit Game", BOARD_HAS_AUX_BUTTON() ? "Start" : "Hyperspace", "Fire", "Thrust", "Hyperspace", &fabgl::FONT_std_16);
  }
  else
  {
    canvas().drawBitmap(0, 0, &AsteroidsSplash);
  }
}

// ----------------------------------------------------------------------------------------
void Asteroids::drawRunning()
{
  drawGameStats();

  myShip.draw();
  for (auto& s : myShots)     s->draw();
  for (auto& a : myAsteroids) a->draw();
  mySaucer.draw();
}

// ----------------------------------------------------------------------------------------
void Asteroids::drawLevelCleared()
{
  drawGameStats();
  drawCenteredText(80, "Level Cleared!", &fabgl::FONT_std_24);

  myShip.draw();
  for (auto& s : myShots) s->draw();
}

// ----------------------------------------------------------------------------------------
void Asteroids::drawGotHit()
{
  drawGameStats();
  for (auto& s : myShots)     s->draw();
  for (auto& a : myAsteroids) a->draw();
  mySaucer.draw();
  myShip.draw();
}

// ----------------------------------------------------------------------------------------
void Asteroids::drawGameOver()
{
  // Game over message & high score (if any)
  drawGameStats();
  for (auto& a : myAsteroids) a->draw();
  mySaucer.draw();
  myShip.draw();

  if (millis() - myStartWaitTime > 1500)
  {
    canvas().setPenColor(cDefaultCol);
    canvas().setBrushColor(Color::Black);
    drawCenteredText(80, "Game Over", &fabgl::FONT_std_24);
    if (myScore >= myHighScore)
    {
      drawCenteredText(150, "New High Score", &fabgl::FONT_std_24);
    }
  }
}

// ----------------------------------------------------------------------------------------
void Asteroids::drawGameStats()
{
  canvas().setGlyphOptions(GlyphOptions().FillBackground(true));
  canvas().setPenColor(cDefaultCol);
  canvas().setBrushColor(Color::Black);

  drawLeftAlignedText(70, 8, String(myScore), &fabgl::FONT_9x15);
  drawCenteredText(7, String(myHighScore), &fabgl::FONT_7x13);

  // Ship 1-5 to the left
  for (int i=0; i<myNumLives && i<5; i++)
  {
    myShip.drawShipShape(Vector2(65-10*i, 30), 0, 0.7, cDefaultCol); 
  }

  // > 5 draw to the right
  for (int i=5; i<myNumLives; i++)
  {
    myShip.drawShipShape(Vector2(5+10*(i-4), 30), 0, 0.7, cDefaultCol); 
  }
}

