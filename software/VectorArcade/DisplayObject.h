#pragma once

#include <fabgl.h>
#include "Theme.h"

// ---- DisplayObject ---------------------------------------------------------------------
// Abstract base class for objects to be displayed.

class DisplayObject
{
  public:
    enum EState
    {
      eActive,
      eNotActive
    };

  public:
    DisplayObject(fabgl::Canvas& canvas);
    DisplayObject(const DisplayObject&) = delete;
    virtual ~DisplayObject() = 0;

    // Update the state of the display object. 'dt' milliseconds have passed since the last call
    virtual void step(unsigned long dt) = 0;
  
    // Draw the object 
    virtual void draw() = 0;
  
    EState getState() const;
    void setState(EState state);
    
    // Returns true if display object is active and has to be processed in the (game) loop 
    bool isActive() const;

    fabgl::Canvas& canvas() { return myCanvas; }
    void drawCenteredText(int y, const String& text, fabgl::FontInfo const * fontInfo = nullptr);
    void drawCenteredText(int y, const char* text, fabgl::FontInfo const * fontInfo = nullptr);
    void drawLeftAlignedText(int x, int y, const String& text, fabgl::FontInfo const * fontInfo = nullptr);
    void drawLeftAlignedText(int x, int y, const char* text, fabgl::FontInfo const * fontInfo = nullptr);
    void drawInstructions(const String& sel, const String& start,
                          const String& a, const String& b, const String& x, 
                          fabgl::FontInfo const * fontInfo = nullptr);
    void drawButton(int x, int y, int length, const String& text, bool isActive);

    // A frame two pixels wide in cDefaultCol, outer edge (x1, y1) - (x2, y2). Drawn as four
    // filled bars: drawRectangle() with pen width 2 leaves a notch at every corner.
    void drawFrame(int x1, int y1, int x2, int y2);

  protected:
    EState myState;
    fabgl::Canvas& myCanvas;
};