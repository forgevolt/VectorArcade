#include "DisplayObject.h"
#include "Board.h"

// ---- DisplayObject ---------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
DisplayObject::DisplayObject(fabgl::Canvas& canvas)
: myState(eActive),
  myCanvas(canvas)
{}

// ----------------------------------------------------------------------------------------
DisplayObject::~DisplayObject()
{}

// ----------------------------------------------------------------------------------------
DisplayObject::EState DisplayObject::getState() const
{
  return myState;
}

// ----------------------------------------------------------------------------------------
void DisplayObject::setState(EState state)
{
  // Ignore invalid state
  if (state == eActive || state == eNotActive)
    myState = state;
}

// ----------------------------------------------------------------------------------------
bool DisplayObject::isActive() const
{
  return (myState == eActive);
}

// ----------------------------------------------------------------------------------------
void DisplayObject::drawCenteredText(int y, const String& text, fabgl::FontInfo const * fontInfo)
{
  drawCenteredText(y, text.c_str(), fontInfo);
}

// ----------------------------------------------------------------------------------------
void DisplayObject::drawCenteredText(int y, const char* text, fabgl::FontInfo const * fontInfo)
{
  if (fontInfo != nullptr)
    myCanvas.selectFont(fontInfo);

  int extent = myCanvas.textExtent(text);
  myCanvas.drawText(myCanvas.getWidth()/2-extent/2, y, text);
}

// ----------------------------------------------------------------------------------------
void DisplayObject::drawLeftAlignedText(int x, int y, const String& text, fabgl::FontInfo const * fontInfo)
{
  drawLeftAlignedText(x, y, text.c_str(), fontInfo);
}

// ----------------------------------------------------------------------------------------
void DisplayObject::drawLeftAlignedText(int x, int y, const char* text, fabgl::FontInfo const * fontInfo)
{
  if (fontInfo != nullptr)
    myCanvas.selectFont(fontInfo);

  myCanvas.drawText(x-myCanvas.textExtent(text), y, text);
}

// ----------------------------------------------------------------------------------------
void DisplayObject::drawInstructions(const String& sel, const String& start,
                                     const String& a, const String& b, const String& x, 
                                     fabgl::FontInfo const * fontInfo)
{
  if (fontInfo != nullptr)
    myCanvas.selectFont(fontInfo);

  myCanvas.setPenWidth(1);

#if BOARD_HAS_AUX_BUTTON()
  // Sel
  myCanvas.setBrushColor(80, 80, 80);
  myCanvas.fillEllipse(35+2, 121+2, 25, 15);
  myCanvas.setBrushColor(Color::Black);
  myCanvas.fillEllipse(35, 121, 25, 15);
  myCanvas.drawEllipse(35, 121, 25, 15);
  myCanvas.moveTo(45, 137);
  myCanvas.lineTo(53, 145);
  myCanvas.lineTo(59, 145);
  myCanvas.drawText(65, 145-8, sel.c_str());

  // Start
  myCanvas.setBrushColor(80, 80, 80);
  myCanvas.fillEllipse(25+75+2, 121+2, 25, 15);
  myCanvas.setBrushColor(Color::Black);
  myCanvas.fillEllipse(25+75, 121, 25, 15);
  myCanvas.drawEllipse(25+75, 121, 25, 15);
  myCanvas.moveTo(110, 111);
  myCanvas.lineTo(118, 103);
  myCanvas.lineTo(124, 103);
  myCanvas.drawText(131, 103-8, start.c_str()); 

  // A
  myCanvas.setBrushColor(80, 80, 80);
  myCanvas.fillEllipse(140+2, 190+2, 20, 20);
  myCanvas.setBrushColor(Color::Black);
  myCanvas.fillEllipse(140, 190, 20, 20);
  myCanvas.drawEllipse(140, 190, 20, 20);
  myCanvas.moveTo(149, 205);
  myCanvas.lineTo(157, 213);
  myCanvas.lineTo(163, 213);
  myCanvas.drawText(169, 213-8, a.c_str());

  // B
  myCanvas.setBrushColor(80, 80, 80);
  myCanvas.fillEllipse(140+60+2, 190+2, 20, 20);
  myCanvas.setBrushColor(Color::Black);
  myCanvas.fillEllipse(140+60, 190, 20, 20);
  myCanvas.drawEllipse(140+60, 190, 20, 20);    
  myCanvas.moveTo(210, 138+35);
  myCanvas.lineTo(218, 130+35);
  myCanvas.lineTo(224, 130+35);
  myCanvas.drawText(231, 130+35-8, b.c_str());

  // X
  myCanvas.setBrushColor(80, 80, 80);
  myCanvas.fillEllipse(140+30+2, 190-40+2, 20, 20);
  myCanvas.setBrushColor(Color::Black);
  myCanvas.fillEllipse(140+30, 190-40, 20, 20);
  myCanvas.drawEllipse(140+30, 190-40, 20, 20);    
  myCanvas.moveTo(180, 138+35-40);
  myCanvas.lineTo(188, 130+35-40);
  myCanvas.lineTo(194, 130+35-40);
  myCanvas.drawText(201, 130+35-40-8, x.c_str());
#else
  static const Point triangle[3]       = { {100, 111}, {100, 131}, {115, 121} };
  static const Point triangleShadow[3] = { {100+2, 111+2}, {100+2, 131+2}, {115+2, 121+2} };

  (void)x;

  // Sel
  myCanvas.setBrushColor(80, 80, 80);
  myCanvas.fillRectangle(25+2, 111+2, 45+2, 131+2);
  myCanvas.setBrushColor(Color::Black);
  myCanvas.fillRectangle(25, 111, 45, 131);
  myCanvas.drawRectangle(25, 111, 45, 131);
  myCanvas.moveTo(35, 137);
  myCanvas.lineTo(43, 145);
  myCanvas.lineTo(49, 145);
  myCanvas.drawText(55, 138, sel.c_str());


  // Start
  myCanvas.setBrushColor(80, 80, 80);
  myCanvas.fillPath(triangleShadow, 3);
  myCanvas.setBrushColor(Color::Black);
  myCanvas.fillPath(triangle, 3);
  myCanvas.drawPath(triangle, 3);
  myCanvas.moveTo(110, 111);
  myCanvas.lineTo(119, 103);
  myCanvas.lineTo(125, 103);
  myCanvas.drawText(131, 97, start.c_str()); 

  // A
  myCanvas.setBrushColor(80, 80, 80);
  myCanvas.fillEllipse(120+2, 190+2, 20, 20);
  myCanvas.setBrushColor(Color::Black);
  myCanvas.fillEllipse(120, 190, 20, 20);
  myCanvas.drawEllipse(120, 190, 20, 20);
  myCanvas.moveTo(129, 205);
  myCanvas.lineTo(137, 213);
  myCanvas.lineTo(143, 213);
  myCanvas.drawText(149, 206, a.c_str());

  // B
  myCanvas.setBrushColor(80, 80, 80);
  myCanvas.fillEllipse(142+2, 155+2, 20, 20);
  myCanvas.setBrushColor(Color::Black);
  myCanvas.fillEllipse(142, 155, 20, 20);
  myCanvas.drawEllipse(142, 155, 20, 20);
  myCanvas.moveTo(142, 138);
  myCanvas.lineTo(151, 130);
  myCanvas.lineTo(157, 130);
  myCanvas.drawText(163, 124, b.c_str());
#endif
}

// ----------------------------------------------------------------------------------------
void DisplayObject::drawButton(int x, int y, int length, const String& text, bool isActive)
{
  myCanvas.setPenWidth(2);
  myCanvas.setLineEnds(LineEnds::None);
  myCanvas.setPenColor(cDefaultCol);
  myCanvas.drawRectangle(x, y-16, x+length, y+16);

  if (isActive == true)
  {
    myCanvas.setBrushColor(cDefaultCol);
    myCanvas.fillRectangle(x+3, y-12, x+length-4, y+13);
    myCanvas.setPenColor(Color::Black);
  }

  // Render the button text
  myCanvas.selectFont(&fabgl::FONT_std_24);
  int extent = myCanvas.textExtent(text.c_str());
  myCanvas.drawText(x+length/2 - extent/2, y-10, text.c_str());  
}
