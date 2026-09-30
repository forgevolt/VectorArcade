#include "LunarLander.h"
#include "Layout.h"

#include "LunarLanderSounds.h"
#if !SOUNDS_FROM_LITTLEFS()
  #include "sounds/RocketEngine.h"
  #include "sounds/FuelAlarm.h"
  #include "sounds/Explosion.h"
#endif

#include "MathUtilities.h"
#include <Streaming.h>

// ---- LunarLanderSounds -----------------------------------------------------------------

#if SOUNDS_FROM_LITTLEFS()
// ----------------------------------------------------------------------------------------
LunarLanderSounds::LunarLanderSounds(SoundEngine& sound)
{
  loadSoundClip(sound, rocketEngine, "/RocketEngine.wav");
  loadSoundClip(sound, fuelAlarm,    "/FuelAlarm.wav");
  loadSoundClip(sound, explosion,    "/Explosion.wav");
  setVolumes();
}
#else
// ----------------------------------------------------------------------------------------
LunarLanderSounds::LunarLanderSounds(SoundEngine&)
: rocketEngine(cSoundRocketEngineWAV),
  fuelAlarm(cSoundFuelAlarmWAV),
  explosion(cSoundExplosionWAV)
{
  setVolumes();
}
#endif

// ----------------------------------------------------------------------------------------
void LunarLanderSounds::setVolumes()
{
  fuelAlarm.setVolume(10);
  explosion.setVolume(30);
}


// ---- LunarLander -----------------------------------------------------------------------

namespace
{

// Start scrolling (up/down/left/right) if distance to the respective edge is lower
const float cBorderLR       = 15;      // Left/right edge distance
const float cBorderLRZoom   = 50;      // Left/right edge distance in zoom mode
const float cBorderTop      = 70;      // Top edge distance
const float cBorderBottom   = 240-100; // Bottom edge distance
const float cMaxWorldY      = Layout::cLunarMaxWorldY; // Lowest point (world coordinates)
const float cDefaultCameraY = 155;     // Y position of the camera if not in zoom mode

const unsigned long cMsgWaitTime   = 5000; // Successful landing and crash messages are visible for 5sec
const unsigned long cMsgRemoveTime = 3000; // Part of the crash message is removed after 3sec

} // namespace


// ----------------------------------------------------------------------------------------
#if BOARD_HAS_IMU()
LunarLander::LunarLander(Menu& menu, MPU6050& imu)
: MenuItem(menu, "Lunar Lander"), myIMU(imu), myIsJoystickMode(true), 
#else
LunarLander::LunarLander(Menu& menu)
: MenuItem(menu, "Lunar Lander"),
#endif
  mySounds(menu.sound()),
  myCamera(canvas().getWidth(), canvas().getHeight()),
  myEagle(canvas(), myCamera, menu.sound(), mySounds),
  mySurface(canvas(), myCamera)
{}

// ----------------------------------------------------------------------------------------
void LunarLander::step(unsigned long dt)
{
  if (isActive() == false)
    return;

  // ---- Query input events
  myInput = myMenu.input().getState();

  // Button 'Sel' ends the game -> back to menu
  if (myInput.sel == true)
  {
    sound().stopAll();
    sound().play(sound().click());
    setState(eNotActive); // End the game
    return;
  }

#if BOARD_HAS_IMU()
  // ---- IMU fast updates (no delays!)
  if (myIsJoystickMode == false || myState == eSelectMode)
    myIMU.update();
#endif

  switch (myState)
  {
#if BOARD_HAS_IMU()
    case eSelectMode:
      stepSelectMode(dt);
      break;

#endif
    case eSelectFuelAndStart:
      stepSelectFuelAndStart(dt);
      break;

    case eRunning:
      stepRunning(dt);
      break;

    case eLanded:
      stepLanded(dt);
      break;

    case eCrashed:
      stepCrashed(dt);
      break;

    case eOutOfFuel:
      stepOutOfFuel(dt);
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
void LunarLander::draw()
{
  if (isActive() == false)
  {
    MenuItem::draw();
    myEagle.drawShape(Vector2(37, myPosY), 8.5, myIsSelected ? Color::Black : cDefaultCol);
  }
  else 
  {
    // Clear screen
    canvas().setBrushColor(Color::Black);
    canvas().clear();

    switch (myState)
    {
#if BOARD_HAS_IMU()
      case eSelectMode:
        drawSelectMode();
        break;

#endif
      case eSelectFuelAndStart:
        drawSelectFuelAndStart();
        break;

      case eRunning:
        drawRunning();
        break;

      case eLanded:
        drawLanded();
        break;

      case eCrashed:
        drawCrashed();
        break;

      case eOutOfFuel:
        drawOutOfFuel();
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
void LunarLander::start()
{
  myState = eGameOver;
  myTotalScore = 0;
  myTimePassed = 0;
  myShowInstructions = false;
  myEagle.reset(true);
  prepareGameOver();

  setState(eActive);
}

// ----------------------------------------------------------------------------------------
void LunarLander::prepareStartGame()
{
  myCamera.setZoom(1.0);
  myCamera.setPos(Vector2(120, 120));

  myEagle.reset(true);
  myEagle.setPos(120, Layout::cLunarStartEagleY);
  myEagle.setScale(40);

  myTotalScore = 0;
  myTimePassed = 0;
#if BOARD_HAS_IMU()
  myIsJoystickMode = true;
#endif
}

// ----------------------------------------------------------------------------------------
void LunarLander::prepareAttempt()
{
  myLandingType = eNone;
  myStartAttemptTime = millis();
  myTimePassed = 0;
  myIsZoomActive = false;

  myCamera.setZoom(Layout::cLunarZoom);
  myCamera.setPos(Vector2(mySurface.getWidth() / 2, cDefaultCameraY));
  
  myEagle.reset(false);
  myEagle.setPos(myCamera.screenToWorldX(cBorderLR), myCamera.screenToWorldY(cBorderTop));
  myEagle.setScale(3);
  myEagle.setVelocity(3.0, 1.2);
  myEagle.setAngle(-90);
  myEagle.applyThrust(0);
  // myEagle.setDebug(true);
  
  mySurface.selectLandingZones();
  mySurface.displayLandingZones(true);  
}

// ----------------------------------------------------------------------------------------
void LunarLander::prepareGameOver()
{
  myCamera.setZoom(Layout::cLunarZoom);
  myCamera.setPos(Vector2(mySurface.getWidth() / 2, cDefaultCameraY));
  
  myEagle.reset(false);
  myEagle.setPos(myCamera.screenToWorldX(cBorderLR), myCamera.screenToWorldY(cBorderTop));
  myEagle.setScale(3);
  myEagle.setVelocity(30.0, 0);
  myEagle.applyThrust(0);
  
  mySurface.displayLandingZones(false);  
}

#if BOARD_HAS_IMU()
// ----------------------------------------------------------------------------------------
void LunarLander::stepSelectMode(unsigned long dt)
{
  if (myIsJoystickMode == false)
  {
    myEagle.applyThrust(mapf(constrain(myIMU.getAngleY(), 20, 45), 20, 45, 0, Eagle::cMaxThrust));
  }
  else 
  {
    myEagle.applyThrust(mapf(constrain(myInput.y, 0, 1000), 0, 1000, 0, Eagle::cMaxThrust));
  }

  if (myInput.a == true || myInput.b == true)
  {
    sound().stopAll();
    sound().play(sound().click());
    
    myState = eSelectFuelAndStart;
  }

  if (myInput.right == true || myInput.left == true)
  {
    myIsJoystickMode = !myIsJoystickMode;
  }
}
#endif

// ----------------------------------------------------------------------------------------
void LunarLander::stepSelectFuelAndStart(unsigned long dt)
{
  if (myInput.start == true)
  {
    sound().play(sound().click());
    prepareAttempt();
    myState = eRunning;
  }
  else if (myInput.a == true)
  {
    sound().play(sound().click());

    // Increments in cStartFuel steps (same as the arcade)
    myEagle.setFuel(myEagle.getFuel() + Eagle::cStartFuel);
  }
}

// ----------------------------------------------------------------------------------------
void LunarLander::stepRunning(unsigned long dt)
{
  // ---- Check for successful landing

  if (floatEquals(myEagle.getAngle(), 0.0f) && fabsf(myEagle.getVelocity().y) < 0.4f && fabsf(myEagle.getVelocity().x) < 0.4f)
  {
    short bonus = mySurface.isOnZone(myEagle.getBBMin()+myEagle.getPos(), myEagle.getBBMax()+myEagle.getPos());
    if (bonus > 0)
    {
      myEagle.hasLanded();
      myAltitude = 0;
      
      // Compute score
      if (fabsf(myEagle.getVelocity().y) < 0.25f)
      {
        myLandingType = eGreat;
        myScore = 50 * bonus; 
        myTotalScore += myScore;        
      }
      else if (fabsf(myEagle.getVelocity().y) < 0.40f)
      {
        myLandingType = eGood;
        myScore = 30 * bonus; 
        myTotalScore += myScore;
      }
      else
      {
        myLandingType = eMarooned;
        myScore = 15 * bonus; 
        myTotalScore += myScore;        
      }

      // ... and change state
      myStartMsgTime = millis();
      myState = eLanded;
      return;
    }
  }

  // ---- Check for crash

  if (mySurface.checkCollison(myEagle.getBBMin()+myEagle.getPos(), myEagle.getBBMax()+myEagle.getPos()) == true)
  {
    myFuelLost = myEagle.getFuel();
    myEagle.hasCrashed();
    myFuelLost -= myEagle.getFuel();

    myScore = 5; 
    myTotalScore += myScore;
   
    myStartMsgTime = millis();
    myState = eCrashed;
    return;
  }
  
  // ---- Check for out of fuel

  if (myEagle.getFuel() <= 0)
  {
    myStartMsgTime = millis();
    myState = eOutOfFuel;
    return;
  }

  // ---- Rotate the lander

  if (myInput.b == true)
  {
    myEagle.setAngle(myEagle.getAngle() + 18);
    myEagle.setFuel(myEagle.getFuel() - 0.45f); // Each rotation reduces fuel by 0.45l
  }  

  if (myInput.a == true)
  {
    myEagle.setAngle(myEagle.getAngle() - 18);
    myEagle.setFuel(myEagle.getFuel() - 0.45f); // Each rotation reduces fuel by 0.45l
  }  

  // ---- Abort landing? (Aux button, Start on boards without Aux)

  if ((BOARD_HAS_AUX_BUTTON() ? myInput.aux : myInput.start) == true)
  {
    myEagle.abort();
  }

  // ---- Zoom in when close to the surface

  myAltitude = min(mySurface.altitude(myEagle.getPos().x+myEagle.getBBMin().x)-myEagle.getPos().y, 
                   mySurface.altitude(myEagle.getPos().x+myEagle.getBBMax().x)-myEagle.getPos().y);

  if (myIsZoomActive == false && myAltitude < 20)
  {
    myIsZoomActive  = true;
    myCameraXBeforeZoom = myCamera.getPos().x;
    myEagleXBeforeZoom = myEagle.getPos().x;
    myZoomTime = millis();

    myCamera.setZoom(7);
    myCamera.setPos(myEagle.getPos().x, myEagle.getPos().y+10);
    myEagle.setScale(1);
  }
  else if (myIsZoomActive == true && myAltitude > 30 && (millis() - myZoomTime) > 500)
  {
    myIsZoomActive = false;
    myCamera.setZoom(Layout::cLunarZoom);
    myCamera.setPos(myCameraXBeforeZoom + myEagle.getPos().x - myEagleXBeforeZoom, cDefaultCameraY);
    myEagle.setScale(3);
  }

  // ---- Move camera when close to the screen border

  // Moving right?
  if (myEagle.getVelocity().x > 0)
  {
    // right edge = right screen edge - border
    float rightEdge = myCamera.screenToWorldX(canvas().getWidth() - (myIsZoomActive ? cBorderLRZoom : cBorderLR));

    // If the eagle has passed the "right edge" of the camera window, then move 
    // the camera so that the eagle remains visible 
    if (myEagle.getPos().x > rightEdge) 
    {
      myCamera.setPos(myCamera.getPos() + Vector2(myEagle.getPos().x - rightEdge, 0));

      // Wrap around?
      if (myCamera.getPos().x > mySurface.getWidth()*1.5f)
      {
        myEagle.setPos(myEagle.getPos()   - Vector2(mySurface.getWidth(), 0));
        myCamera.setPos(myCamera.getPos() - Vector2(mySurface.getWidth(), 0));
      }
    }
  }
  
  // Moving left? -> same procedure as above when moving right
  else if (myEagle.getVelocity().x < 0)
  {
    float leftEdge = myCamera.screenToWorldX(myIsZoomActive ? cBorderLRZoom : cBorderLR);
    if (myEagle.getPos().x < leftEdge) 
    {
      myCamera.setPos(myCamera.getPos() + Vector2(myEagle.getPos().x - leftEdge, 0));

      if (myCamera.getPos().x < -mySurface.getWidth()/2)
      {
        myEagle.setPos(myEagle.getPos()   + Vector2(mySurface.getWidth(), 0));
        myCamera.setPos(myCamera.getPos() + Vector2(mySurface.getWidth(), 0));
      }
    }
  }

  // Moving up?
  if (myEagle.getVelocity().y < 0)
  {
    // Eagle is close to the game statistics (screen coordinates) ... ?
    if (myEagle.getPos().y < myCamera.screenToWorldY(cBorderTop))
    {
      // ... but not at the top of the "world" (world coordinates)?
      if (myEagle.getPos().y > cBorderTop)
      {
        myCamera.setPos(myCamera.getPos() + Vector2(0, myEagle.getPos().y - myCamera.screenToWorldY(cBorderTop)));
      }
      // Stop movement since we cannot move up further
      else
      {
        myEagle.setPos(myEagle.getPos().x, myCamera.screenToWorldY(cBorderTop));
        myEagle.setVelocity(myEagle.getVelocity().x, 0);
        myEagle.stopAbort();
      }
    }
  }

  // Moving down?
  else if (myEagle.getVelocity().y > 0)
  {
    // Eagle is close to the bottom of the screen (screen coordinates) ... ?
    if (myEagle.getPos().y > myCamera.screenToWorldY(cBorderBottom))
    {
      // ... but not at the bottom of the "world" (world coordinates)?
      if (myCamera.screenToWorldY(canvas().getHeight()) < cMaxWorldY)
      {
        myCamera.setPos(myCamera.getPos() + Vector2(0, myEagle.getPos().y - myCamera.screenToWorldY(cBorderBottom)));
      }
      // Stop movement since we cannot move down further (not actually relevant since crash/landing happens first)
      else if (myEagle.getPos().y > canvas().getHeight())
      {
        myEagle.setPos(myEagle.getPos().x, canvas().getHeight());
        myEagle.setVelocity(myEagle.getVelocity().x, 0);
      }
    }    
  }

  // ---- Fire rocket engine and update the eagle

#if BOARD_HAS_IMU()
  if (myIsJoystickMode == false)
  {
    myEagle.applyThrust(mapf(constrain(myIMU.getAngleY(), 20, 45), 20, 45, 0, Eagle::cMaxThrust));
  }
  else 
  {
    myEagle.applyThrust(mapf(constrain(myInput.y, 0, 1000), 0, 1000, 0, Eagle::cMaxThrust));
  }
#else
  myEagle.applyThrust(mapf(constrain(myInput.y, 0, 1000), 0, 1000, 0, Eagle::cMaxThrust));
#endif
  myEagle.step(dt);

  myTimePassed = millis()-myStartAttemptTime;
}

// ----------------------------------------------------------------------------------------
void LunarLander::stepLanded(unsigned long dt)
{
  // Show message for 5sec
  if (millis() - myStartMsgTime > cMsgWaitTime)
  {
    prepareAttempt();
    myState = eRunning;
  }
}

// ----------------------------------------------------------------------------------------
void LunarLander::stepCrashed(unsigned long dt)
{
  // Show message for 5sec
  if (millis() - myStartMsgTime > cMsgWaitTime)
  {
    if (myEagle.getFuel() > 0)
    {
      prepareAttempt();
      myState = eRunning;
    }
    else
    {
      prepareGameOver();
      myShowInstructions = false;
      myState = eGameOver;
    }
  }

  myEagle.step(dt);
}

// ----------------------------------------------------------------------------------------
void LunarLander::stepOutOfFuel(unsigned long dt)
{
  // Show message for 5sec
  if (millis() - myStartMsgTime > cMsgWaitTime)
  {
    prepareGameOver();
    myState = eGameOver;
    myShowInstructions = false;
  }
  else if (mySurface.checkCollison(myEagle.getBBMin()+myEagle.getPos(), myEagle.getBBMax()+myEagle.getPos()) == true)
  {
    myEagle.hasCrashed();
    myStartMsgTime = millis();
    myFuelLost = 0;
    myState = eCrashed;
    return;
  }

  myEagle.step(dt);
}

// ----------------------------------------------------------------------------------------
void LunarLander::stepGameOver(unsigned long dt)
{
  // ---- Check buttons

  if (myInput.a == true || myInput.b == true || myInput.aux == true || myInput.start)
  {
    sound().play(sound().click());
    prepareStartGame();
#if BOARD_HAS_IMU()
    myState = eSelectMode;
#else
    myState = eSelectFuelAndStart;
#endif
    return;
  }

  if (myShowInstructions == true)
  {
    // Show instruction screen for cMsgWaitTime time
    if (millis() - myStartMsgTime > cMsgWaitTime)
    {
      myShowInstructions = false;
      prepareGameOver();
    }
  }
  else
  {
  // ---- "Crash" -> start over

    if (mySurface.checkCollison(myEagle.getBBMin()+myEagle.getPos(), myEagle.getBBMax()+myEagle.getPos()) == true)
    {
      myShowInstructions = true;
      myStartMsgTime = millis();
    }

    myAltitude = min(mySurface.altitude(myEagle.getPos().x+myEagle.getBBMin().x)-myEagle.getPos().y, 
                     mySurface.altitude(myEagle.getPos().x+myEagle.getBBMax().x)-myEagle.getPos().y);

    // ---- Move camera when close to the screen border

    // Moving right?
    if (myEagle.getVelocity().x > 0)
    {
      // right edge = right screen edge - border
      float rightEdge = myCamera.screenToWorldX(canvas().getWidth() - cBorderLRZoom);

      // If the eagle has passed the "right edge" of the camera window, then move 
      // the camera so that the eagle remains visible 
      if (myEagle.getPos().x > rightEdge) 
      {
        myCamera.setPos(myCamera.getPos() + Vector2(myEagle.getPos().x - rightEdge, 0));

        // Wrap around?
        if (myCamera.getPos().x > mySurface.getWidth()*1.5f)
        {
          myEagle.setPos(myEagle.getPos()   - Vector2(mySurface.getWidth(), 0));
          myCamera.setPos(myCamera.getPos() - Vector2(mySurface.getWidth(), 0));
        }
      }
    }

    myEagle.step(dt);
  }
}

#if BOARD_HAS_IMU()
// ----------------------------------------------------------------------------------------
void LunarLander::drawSelectMode()
{
  canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
  canvas().setPenColor(cDefaultCol);
  canvas().selectFont(&fabgl::FONT_std_24);

  drawCenteredText(10, myMenuName);
  canvas().drawLine(0, 40, canvas().getWidth(), 40);
  drawCenteredText(50, "Select mode");

  drawButton(30,  180, 60, "Gyro", !myIsJoystickMode);
  drawButton(canvas().getWidth()-30-60, 180, 60, "Joy",  myIsJoystickMode);
  
  myEagle.setColor(cDefaultCol);
  // myEagle.setDebug(true);
  myEagle.draw();
}
#endif

// ----------------------------------------------------------------------------------------
void LunarLander::drawSelectFuelAndStart()
{
  canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
  canvas().setPenColor(cDefaultCol);
  canvas().selectFont(&fabgl::FONT_std_24);

  drawCenteredText(10, myMenuName);
  canvas().drawLine(0, 40, canvas().getWidth(), 40);
  drawCenteredText(60, "Add fuel by");
  drawCenteredText(90, "pressing button 'A'");
  drawCenteredText(120, "and 'Start' to play");

  char buffer[15];
  snprintf(buffer, sizeof(buffer), "%d l", (int)myEagle.getFuel());
  drawCenteredText(170, buffer);
}

// ----------------------------------------------------------------------------------------
void LunarLander::drawRunning()
{
  myEagle.setColor(cDefaultCol);
  myEagle.draw(); 
  mySurface.draw();

  canvas().setGlyphOptions(GlyphOptions().FillBackground(true));
  canvas().setPenColor(cDefaultCol);
  canvas().setBrushColor(Color::Black);
  canvas().selectFont(&fabgl::FONT_6x8);

  drawGameStats();

  // ---- Low Fuel

  // If low on fuel, then display message (blink rate 350ms)
  if (myEagle.getFuel() < Eagle::cFuelLow && myEagle.getFuel() > 0 && millis() / 350 % 2)
  {
    drawCenteredText(70, "LOW ON FUEL");
  }

  canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
}

// ----------------------------------------------------------------------------------------
void LunarLander::drawLanded()
{
  myEagle.setColor(cDefaultCol);
  myEagle.draw();

  mySurface.displayLandingZones(false);  
  mySurface.draw();

  canvas().setGlyphOptions(GlyphOptions().FillBackground(true));
  canvas().setPenColor(cDefaultCol);
  canvas().setBrushColor(Color::Black);
  canvas().selectFont(&fabgl::FONT_6x8);

  drawGameStats();

  char buffer[15];
  snprintf(buffer, sizeof(buffer), " %d POINTS ", myScore);

  switch (myLandingType)
  {
    case eGreat:
      drawCenteredText(70, " CONGRATULATIONS ");    
      drawCenteredText(85, " THAT WAS A GREAT LANDING ");    
      drawCenteredText(105, buffer);
      break;

    case eGood:
      drawCenteredText(70, " CONGRATULATIONS ");
      drawCenteredText(85, " YOU HAVE LANDED ");
      drawCenteredText(105, buffer);
      break;

    case eMarooned:
      drawCenteredText(70, " YOU HAVE LANDED HARD ");
      drawCenteredText(85, " YOU ARE HOPELESSLY MAROONED ");    
      drawCenteredText(105, buffer);    
      break;

    case eNone:
      break;    // not reached - drawLanded() only runs after a landing
  }

  canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
}

// ----------------------------------------------------------------------------------------
void LunarLander::drawCrashed()
{
  myEagle.setColor(cDefaultCol);
  myEagle.draw();

  mySurface.displayLandingZones(false);  
  mySurface.draw();

  canvas().setGlyphOptions(GlyphOptions().FillBackground(true));
  canvas().setPenColor(cDefaultCol);
  canvas().setBrushColor(Color::Black);
  canvas().selectFont(&fabgl::FONT_6x8);

  drawGameStats();
  
  char buffer[25];

  if (millis() - myStartMsgTime < cMsgRemoveTime && myFuelLost > 0)
  {
    drawCenteredText(70, " AUXILIARY FUEL TANK DESTROYED ");
    snprintf(buffer, sizeof(buffer), " %d FUEL UNITS LOST ", (int)myFuelLost);
    drawCenteredText(85, buffer);
  }

  drawCenteredText(110, " THERE WERE NO SURVIVORS ");
  snprintf(buffer, sizeof(buffer), " %d POINTS ", myScore);
  drawCenteredText(135, buffer);

  canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
}

// ----------------------------------------------------------------------------------------
void LunarLander::drawOutOfFuel()
{
  myEagle.setColor(cDefaultCol);
  myEagle.draw();

  mySurface.displayLandingZones(false);  
  mySurface.draw();

  canvas().setGlyphOptions(GlyphOptions().FillBackground(true));
  canvas().setPenColor(cDefaultCol);
  canvas().setBrushColor(Color::Black);
  canvas().selectFont(&fabgl::FONT_6x8);

  drawGameStats();
  
  drawCenteredText(70, " OUT OF FUEL ");

  canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
}

// ----------------------------------------------------------------------------------------
void LunarLander::drawGameOver()
{
  if (myShowInstructions == true)
  {
    canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
    canvas().setPenColor(cDefaultCol);
    canvas().selectFont(&fabgl::FONT_std_24);

    drawCenteredText(10, myMenuName);
    canvas().drawLine(0, 40, canvas().getWidth(), 40);
    drawCenteredText(55, "Press a button to start");

    drawInstructions("Quit Game", BOARD_HAS_AUX_BUTTON() ? "Start" : "Abort Landing", "Rotate Left", "Rotate Right", "Abort Landing", &fabgl::FONT_std_16);
  }
  else 
  {
    myEagle.setColor(cDefaultCol);
    myEagle.draw();

    mySurface.displayLandingZones(false);  
    mySurface.draw();

    canvas().setGlyphOptions(GlyphOptions().FillBackground(true));
    canvas().setPenColor(cDefaultCol);
    canvas().setBrushColor(Color::Black);
    canvas().selectFont(&fabgl::FONT_6x8);

    drawGameStats();

    // Lander flies over the surface until user presses a button
    drawCenteredText(70, " GAME OVER ");
    if (millis() / 500 % 2)
      drawCenteredText(95, " PRESS A BUTTON ");

    canvas().setGlyphOptions(GlyphOptions().FillBackground(false));
  }
}

// ----------------------------------------------------------------------------------------
void LunarLander::drawGameStats()
{
  const int cLeft  = Layout::cLunarStatsLeft;
  const int cRight = Layout::cLunarStatsRight;

  // ---- Left side with score, time and fuel

  canvas().drawTextFmt(cLeft, 10, "SCORE  %04d", myTotalScore);
  canvas().drawTextFmt(cLeft, 25, "TIME  %02d:%02d", myTimePassed / 1000 / 60, myTimePassed / 1000 % 60);
  canvas().drawTextFmt(cLeft, 40, "FUEL   %04d", (int) lroundf(myEagle.getFuel()));

  // ---- Altitude

  canvas().drawTextFmt(cRight, 10, "ALTITUDE %4d", (int) lroundf(myAltitude*10));

  // ---- H Speed

  int horizontalVelocity = lroundf(myEagle.getVelocity().x * 35);

  canvas().drawTextFmt(cRight, 25, "H SPEED  %4d", abs(horizontalVelocity));

  // Horizontal arrow to indicate direction
  if (horizontalVelocity != 0)
  {
    canvas().setPenWidth(1);
    canvas().drawLine(cRight+85, 28, cRight+95, 28);
    if (horizontalVelocity < 0)
    {
      // Arrow left
      canvas().moveTo(cRight+85, 28);
      canvas().lineTo(cRight+88, 25);
      canvas().lineTo(cRight+88, 31);
      canvas().lineTo(cRight+85, 28);
    }
    else
    {
      // Arrow right
      canvas().moveTo(cRight+95, 28);
      canvas().lineTo(cRight+92, 25);
      canvas().lineTo(cRight+92, 31);
      canvas().lineTo(cRight+95, 28);
    }
  }

  // ---- V Speed

  int verticalVelocity = lroundf(myEagle.getVelocity().y * 35);

  canvas().drawTextFmt(cRight, 40, "V SPEED  %4d", abs(verticalVelocity));

  // Vertical arrow to indicate direction
  if (verticalVelocity != 0)
  {
    canvas().setPenWidth(1);
    canvas().drawLine(cRight+90, 38, cRight+90, 48);
    if (verticalVelocity < 0)
    {
      // Arrow up
      canvas().moveTo(cRight+90, 38);
      canvas().lineTo(cRight+87, 41);
      canvas().lineTo(cRight+93, 41);
      canvas().lineTo(cRight+90, 38);
    }
    else 
    {
      // Arrow down
      canvas().moveTo(cRight+90, 48);
      canvas().lineTo(cRight+87, 45);
      canvas().lineTo(cRight+93, 45);
      canvas().lineTo(cRight+90, 48);
    }
  }
}
