#pragma once
#include "MathUtilities.h"

// ---- Camera ----------------------------------------------------------------------------
// This code implements a simple camera that transforms world coordinates into screen 
// coordinates and vice versa. The camera can be moved and zoomed. The worldToScreen 
// function converts a position in the game world into a position on the screen, while 
// screenToWorld does the opposite.


class Camera 
{
  public:
 
    Camera(int width, int height) 
    : myHalfWidth(width / 2.0f),
      myHalfHeight(height / 2.0f), 
      myPos(myHalfWidth, myHalfHeight),
      myZoom(1.0f)
    {}

    // Center position in world coordinates
    void setPos(const Vector2& pos) { myPos = pos;   }
    void setPos(float x, float y)   { myPos.x = x; myPos.y = y; }
    const Vector2& getPos() const   { return myPos;  }

    // Zoom factor, default 1
    void setZoom(float zoom)        { myZoom = zoom; }

    Vector2 worldToScreen(const Vector2& worldPos) 
    {
      return Vector2((worldPos.x - myPos.x) * myZoom + myHalfWidth,
                     (worldPos.y - myPos.y) * myZoom + myHalfHeight);
    }

    int worldToScreenX(float worldX)
    {
      return (worldX - myPos.x) * myZoom + myHalfWidth;
    } 
 
    int worldToScreenY(float worldY)
    {
      return (worldY - myPos.y) * myZoom + myHalfHeight;
    } 

    Vector2 screenToWorld(const Vector2& screenPos) 
    {
      return Vector2((screenPos.x - myHalfWidth)  / myZoom + myPos.x,
                     (screenPos.y - myHalfHeight) / myZoom + myPos.y);
    }

    float screenToWorldX(int screenX)
    {
      return (screenX - myHalfWidth) / myZoom + myPos.x;
    }

    float screenToWorldY(int screenY)
    {
      return (screenY - myHalfHeight) / myZoom + myPos.y;
    }

  private:
    float myHalfWidth;   // Screen dimensions / 2
    float myHalfHeight;  // Screen dimensions / 2
    Vector2 myPos;       // Position of the camera
    float myZoom;        // Zoom factor, 1 = no zoom
};
