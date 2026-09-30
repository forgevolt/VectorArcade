#pragma once

#include "DisplayObject.h"
#include "Camera.h"
#include "Eagle.h"

// ---- LunarSurface ----------------------------------------------------------------------
// The moon surface with the landing area

class LunarSurface : public DisplayObject
{
  public:
    LunarSurface(fabgl::Canvas& canvas, Camera& camera);
    ~LunarSurface() override {};

    // From class DisplayObject
    void step(unsigned long dt) override;
    void draw() override;

    // Choose 4 landing zones as targets
    void selectLandingZones();

    // true: draw() displays bonus level of the selected landing zones
    void displayLandingZones(bool flag) { myDisplayLandingZones = flag; }

    // Returns true if the bounding box (of the eagle/lander) overlaps with the surface 
    bool checkCollison(const Vector2& bbMin, const Vector2& bbMax);

    // Returns a value > 0 if the bounding box (of the eagle/lander) is on one of the landing pads. 
    // The return value is out of [1, 5], no bonus = 1.
    short isOnZone(const Vector2& bbMin, const Vector2& bbMax);

    // Returns altitude of lunar surface at 'x'  
    float altitude(float x);

    // Returns the width of the lunar surface in world coordinates. The surface repeats 
    // endlessly to the left and right with this period, so the camera may be anywhere. The 
    // game keeps the camera within a few periods (wrap around) by moving it and the eagle 
    // by exactly this width, which does not change the view.
    float getWidth() const;

  private:
    // Calls f(px, py, qx, qy) for every line of the repeated surface that overlaps the
    // world range [x0, x1], from left to right
    template <typename F> void forEachSegment(float x0, float x1, F f) const;

    // Returns index i of the line (cSurface point i to point i+1) that contains x, 0 <= x <= width
    static int segmentAt(float x);

    // Returns true if the bounding box (of the eagle/lander) overlaps with a single line 
    bool checkCollison(float px, float py, float qx, float qy, const Vector2& bbMin, const Vector2& bbMax);

    // Compute the intersection (y) of line (px, py) (qx, qy) with y-axis at x
    float intersect(float px, float py, float qx, float qy, float x);

  private:
    Camera& myCamera;
    int myLZSet;                 // Index of the set of landing zones (4) that are displayed
    bool myDisplayLandingZones;  
};