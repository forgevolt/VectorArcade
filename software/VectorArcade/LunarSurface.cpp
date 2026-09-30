#include "esp32-hal.h"
#include "LunarSurface.h"
#include <Streaming.h>

// ---- LunarSurface ----------------------------------------------------------------------

namespace
{

const float cSurfaceWidth = 240;
constexpr float cSurface[] = {
      0.000, 186.246,   2.402, 186.246,   2.683, 188.070,   4.500, 188.070,   4.788, 189.894,   5.810, 189.894,   6.360, 195.598,   7.499, 200.854,   7.599, 203.000,   8.582, 206.366,
     11.616, 208.417,  12.257, 212.092,  13.454, 213.716,  15.291, 215.724,  17.490, 215.724,  19.009, 213.673,  20.846, 210.468,  22.769, 208.289,  24.606, 206.451,  25.632, 204.571,  
     27.512, 202.948,  28.367, 200.982,  30.204, 199.999,  32.127, 197.264,  34.007, 196.965,  35.759, 195.598,  38.000, 195.598,  39.477, 195.854,  41.570, 197.350,  41.955, 199.187,  
     43.365, 200.939,  43.750, 202.819,  45.331, 204.700,  45.673, 206.409,  47.040, 208.460,  49.300, 208.460,  50.416, 204.486,  51.869, 203.717,  53.279, 204.614,  54.603, 206.451,  
     55.501, 210.126,  58.620, 212.306,  58.834, 217.561,  60.278, 223.009,  60.378, 224.700,  61.991, 226.425,  66.275, 226.425,  67.678, 224.822,  69.730, 222.988,  70.627, 221.151,  
     72.379, 219.399,  73.362, 217.519,  75.371, 216.536,  76.292, 211.913,  77.293, 208.332,  79.174, 207.392,  81.054, 205.554,  82.806, 202.948,  84.643, 199.187,  85.583, 195.427,  
     87.463, 193.718,  88.574, 191.795,  88.873, 189.768,  90.284, 187.949,  92.500, 187.949,  94.091, 186.325,  95.830, 183.176,  97.804, 180.826,  98.791, 177.159,  99.405, 171.563,  
     99.605, 167.753, 100.174, 164.113, 101.482, 159.563, 103.530, 158.880, 103.871, 157.060, 105.000, 155.000, 107.300, 155.000, 107.578, 156.992, 109.999, 156.992, 110.146, 158.872, 
    111.094, 159.051, 112.232, 159.676, 113.861, 161.532, 114.791, 165.307, 117.692, 166.956, 120.194, 175.431, 122.128, 175.829, 124.062, 177.137, 124.460, 178.843, 125.938, 180.777, 
    126.280, 182.540, 127.548, 184.590, 129.806, 184.590, 130.204, 189.991, 131.369, 195.337, 131.569, 197.219, 133.750, 197.219, 133.922, 199.119, 135.399, 201.033, 136.244, 204.524, 
    137.257, 206.382, 139.340, 208.353, 139.453, 213.815, 139.918, 219.443, 141.118, 221.195, 141.423, 222.661, 142.828, 224.772, 145.056, 224.772, 145.578, 228.581, 146.716, 230.425, 
    150.488, 234.149, 165.330, 234.149, 166.198, 228.567, 168.058, 224.738, 169.014, 221.247, 170.027, 221.247, 171.491, 217.306, 172.899, 216.518, 174.701, 213.646, 176.671, 213.477, 
    177.982, 212.020, 180.314, 212.020, 181.000, 214.000, 182.189, 216.799, 183.203, 220.177, 183.541, 222.261, 185.005, 224.006, 186.637, 224.907, 187.876, 226.653, 188.777, 230.200, 
    191.725, 232.076, 195.000, 232.076, 198.912, 232.621, 202.854, 233.972, 210.000, 233.972, 211.294, 230.198, 212.031, 224.907, 212.131, 221.229, 214.419, 221.229, 215.016, 217.362, 
    216.874, 215.617, 217.831, 213.871, 218.225, 212.013, 219.357, 210.177, 221.688, 210.177, 222.561, 205.482, 223.462, 201.765, 225.432, 200.808, 227.290, 202.835, 231.000, 202.835, 
    232.865, 201.090, 233.146, 197.317, 234.666, 192.531, 236.581, 191.968, 237.144, 189.941, 238.495, 188.083, 240.000, 186.246  
};

constexpr int cNumPoints = sizeof(cSurface) / sizeof(float) / 2;

struct LandingZones
{
  float x1, x2, y;
  short bonus;
};

constexpr LandingZones cLandingZones[] = {
    {   0.000,   2.402, 186.246, 4 }, // 0
    {  15.291,  17.490, 215.724, 3 }, // 1
    {  35.759,  38.000, 195.598, 4 }, // 2
    {  47.040,  49.300, 208.460, 4 }, // 3
    {  61.991,  66.275, 226.425, 2 }, // 4
    {  90.284,  92.500, 187.949, 5 }, // 5
    { 105.000, 107.300, 155.000, 4 }, // 6
    { 107.578, 109.999, 156.992, 4 }, // 7
    { 127.548, 129.806, 184.590, 4 }, // 8
    { 131.569, 133.750, 197.219, 5 }, // 9
    { 142.828, 145.056, 224.772, 4 }, // 10
    { 150.488, 165.330, 234.149, 2 }, // 11
    { 177.982, 180.314, 212.020, 5 }, // 12
    { 191.725, 195.000, 232.076, 2 }, // 13
    { 202.854, 210.000, 233.972, 2 }, // 14
    { 212.131, 214.419, 221.229, 4 }, // 15
    { 219.357, 221.688, 210.177, 4 }, // 16
    { 227.290, 231.000, 202.835, 3 }  // 17
};

constexpr int cLandingZonesSet[] = {
  4,  12, 13, 17,
  1,  5,  11, 14,
  0,  6,  11, 14,
  2,  4,  9,  13,
  2,  3,  4,  13,
  5,  9,  11, 14,
  1,  4,  5,  13,
  4,  9,  11, 14,
  11, 12, 14, 17,
  2,  3,  11, 14,
  4,  9,  12, 13
};

// Landing zone markings and stars are drawn up to this distance (world coordinates) outside 
// the screen, so that zones and their bonus text are also shown when partly visible
constexpr float cDrawMargin = 20;

constexpr float cStars[] = {
    7.721,  70.709,
   15.299, 166.196,
   45.203, 122.030,
   52.699,  70.652,
   60.255, 173.570,
   82.779,  85.279,
  105.219, 129.447, 
  135.316, 100.096,
  142.904, 195.497,
  157.805, 166.191,
  172.788, 70.698,
  180.295, 180.751,
  187.857, 114.703,
  217.973, 173.385,
  232.804,  63.335
};

} // namespace

// ----------------------------------------------------------------------------------------
LunarSurface::LunarSurface(fabgl::Canvas& canvas, Camera& camera)
: DisplayObject(canvas),
  myCamera(camera),
  myLZSet(0),
  myDisplayLandingZones(true)
{}

// ----------------------------------------------------------------------------------------
template <typename F> 
void LunarSurface::forEachSegment(float x0, float x1, F f) const
{
  // Start with the copy of the surface that contains x0 and walk to the right, continuing
  // with the next copy at the end of the surface
  float offset = floorf(x0 / cSurfaceWidth) * cSurfaceWidth;
  int i = segmentAt(x0 - offset);

  while (cSurface[2*i]+offset <= x1)
  {
    f(cSurface[2*i]+offset,   cSurface[2*i+1], 
      cSurface[2*i+2]+offset, cSurface[2*i+3]);

    if (++i == cNumPoints-1)
    {
      i = 0;
      offset += cSurfaceWidth;
    }
  }
}

// ----------------------------------------------------------------------------------------
int LunarSurface::segmentAt(float x)
{
  // Binary search for the last point with cSurface x <= x
  int lo = 0;
  int hi = cNumPoints-2;
  while (lo < hi)
  {
    int mid = (lo + hi + 1) / 2;
    if (cSurface[2*mid] <= x)
      lo = mid;
    else
      hi = mid - 1;
  }
  return lo;
}

// ----------------------------------------------------------------------------------------
void LunarSurface::step(unsigned long dt)
{
  if (isActive() == false)
    return;
}

// ----------------------------------------------------------------------------------------
void LunarSurface::draw()
{
  if (isActive() == false)
    return;

  canvas().setPenColor(cDefaultCol);
  canvas().setPenWidth(1);
  canvas().setGlyphOptions(GlyphOptions().FillBackground(false));

  // The lunar surface (cSurface) is cSurfaceWidth wide and repeats endlessly to the left 
  // and right. Only the part between the left and right screen edge is drawn.
  float left  = myCamera.screenToWorldX(0);
  float right = myCamera.screenToWorldX(canvas().getWidth());

  // ---- Surface
  bool first = true;
  forEachSegment(left, right, [&](float px, float py, float qx, float qy)
  {
    if (first)
    {
      canvas().moveTo(myCamera.worldToScreenX(px), myCamera.worldToScreenY(py));
      first = false;
    }
    canvas().lineTo(myCamera.worldToScreenX(qx), myCamera.worldToScreenY(qy));
  });

  // ---- Landing zones and stars of each visible copy of the surface
  bool on = (millis() / 350) % 2;

  for (float offset = floorf((left - cDrawMargin) / cSurfaceWidth) * cSurfaceWidth; 
       offset <= right + cDrawMargin; 
       offset += cSurfaceWidth)
  {
    // --- Bonus landing zones
    if (myDisplayLandingZones == true)
    {
      canvas().setPenColor(210, 210, 210);
      canvas().setBrushColor(210, 210, 210);

      for (int i=0; i<4; i++)
      {
        auto lz = cLandingZones[cLandingZonesSet[myLZSet*4 + i]];
        if (lz.x2+offset < left - cDrawMargin || lz.x1+offset > right + cDrawMargin)
          continue;

        canvas().setPenWidth(2);
        canvas().drawLine(myCamera.worldToScreenX(lz.x1+offset), myCamera.worldToScreenY(lz.y),
                          myCamera.worldToScreenX(lz.x2+offset), myCamera.worldToScreenY(lz.y));

        if (on)
        {
          char bonusText[8];
          snprintf(bonusText, sizeof(bonusText), "%dx", lz.bonus);

          // Bonus text is centered below the landing zone
          canvas().selectFont(&fabgl::FONT_7x14);
          canvas().drawText(myCamera.worldToScreenX(lz.x1+offset + (lz.x2 - lz.x1)/2) // + half size of pad
                            - canvas().textExtent(bonusText)/2,                       // - half extent of text
                            myCamera.worldToScreenY(lz.y)+2, 
                            bonusText);
        }
      }    
    }

    // ---- Stars
    canvas().setPenColor(80, 80, 80);

    for (size_t i=0; i<sizeof(cStars)/sizeof(float); i+=2)
    {
      if (cStars[i]+offset < left || cStars[i]+offset > right)
        continue;

      canvas().setPixel(myCamera.worldToScreenX(cStars[i]+offset), 
                        myCamera.worldToScreenY(cStars[i+1]));
    }
  }
}

// ----------------------------------------------------------------------------------------
void LunarSurface::selectLandingZones()
{
  myLZSet = random(0, sizeof(cLandingZonesSet)/sizeof(int)/4);
}

// ----------------------------------------------------------------------------------------
bool LunarSurface::checkCollison(const Vector2& bbMin, const Vector2& bbMax)
{
  // Only the lines below the bounding box can collide
  bool hit = false;
  forEachSegment(bbMin.x, bbMax.x, [&](float px, float py, float qx, float qy)
  {
    if (hit == false && checkCollison(px, py, qx, qy, bbMin, bbMax))
      hit = true;
  });

  return hit;
}

// ----------------------------------------------------------------------------------------
bool LunarSurface::checkCollison(float px, float py, float qx, float qy, 
                                 const Vector2& bbMin, const Vector2& bbMax) 
{
  // Line left of BB?
  if (px < bbMin.x && qx < bbMin.x)
    return false;

  // Line to the right of BB?
  else if (px > bbMax.x && qx > bbMax.x)
    return false;

  // BB above line?
  else if (py > bbMax.y && qy > bbMax.y)
    return false;

  // BB intersects with line! A line entirely above the BB counts as well: the lander is then
  // below the surface, e.g. after a large step, and must still be detected as a crash.
  return true;
} 

// ----------------------------------------------------------------------------------------
short LunarSurface::isOnZone(const Vector2& bbMin, const Vector2& bbMax)
{
  // A landing zone never crosses the end of the surface, so the eagle can only be on a zone
  // of the copy that contains bbMin.x
  float offset = floorf(bbMin.x / cSurfaceWidth) * cSurfaceWidth;

  for (int i=0; i<(int)(sizeof(cLandingZones)/sizeof(LandingZones)); i++)
  {
    if ((bbMin.x - (cLandingZones[i].x1+offset)) >= 0 && (cLandingZones[i].x2+offset-bbMax.x) >= 0 && fabsf(bbMax.y - cLandingZones[i].y) < 0.05f)
    {
      // Bonus?
      for (int j=0; j<4; j++)
      {
        if (cLandingZonesSet[myLZSet*4 + j] == i)
        {
          // ... yes, return bonus
          return cLandingZones[cLandingZonesSet[myLZSet*4 + j]].bonus;
        }
      }
      // No bonus, default = 1
      return 1;
    }
  }

  return 0;
}

// ----------------------------------------------------------------------------------------
float LunarSurface::altitude(float x)
{
  float offset = floorf(x / cSurfaceWidth) * cSurfaceWidth;
  int i = segmentAt(x - offset);

  return intersect(cSurface[2*i]+offset,   cSurface[2*i+1], 
                   cSurface[2*i+2]+offset, cSurface[2*i+3], x);
}

// ----------------------------------------------------------------------------------------
float LunarSurface::getWidth() const
{
  return cSurfaceWidth;
}

// ----------------------------------------------------------------------------------------
float LunarSurface::intersect(float px, float py, float qx, float qy, float x)
{
  // Compute intersection point (see https://www.geeksforgeeks.org/line-intersection-in-cpp/)

  // Line 1: (px, py) (qx, qy)
  // Line 2: (x, 0)   (x, canvas().height())

  float height = canvas().getHeight();

  float a1 = qy - py;
  float b1 = px - qx;
  float c1 = a1 * px + b1 * py;

  // float a2 = height;   // height - 0    (folded into the expressions below)
  // float b2 = 0;           // x - x
  float c2 = height * x;  // height * x + 0 * 0

  float determinant = -height * b1; // a1 * 0 - height * b1

  // No check for determinant == 0 needed, since method is called when precondition 
  // for an intersection is met
  return (a1 * c2 - height * c1) / determinant;
}
