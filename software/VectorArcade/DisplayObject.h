#pragma once

#include <fabgl.h>

// ---- DisplayObject ---------------------------------------------------------------------
// Abstract base class for objects to be displayed.

const RGB888 cDefaultCol (170, 170, 170); // Default color for text and border

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

  protected:
    EState myState;
    fabgl::Canvas& myCanvas;
};