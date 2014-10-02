#ifndef _TAREA2_COLOR_HPP
#define _TAREA2_COLOR_HPP

#include "Vector2.hpp" // For closeTo

namespace Tarea2
{
/**
 * RGBA color.
 */
class Color
{
public:
    Color(float r=0.0f, float g=0.0f, float b=0.0f, float a=1.0f)
        : r(r), g(g), b(b), a(a) {}
    ~Color() {}

    float r, g, b, a;

public:
    Color applyIntensity(float intensity) const
    {
        return Color(r*intensity, g*intensity, b*intensity, a);
    }

public:
    bool closeTo(const Color &c) const
    {
        return Tarea2::closeTo(r, c.r) && Tarea2::closeTo(g, c.g) &&
               Tarea2::closeTo(b, c.b) && Tarea2::closeTo(a, c.a);
    }

public:
    static Color red()
    {
        return Color(1.0f, 0.0f, 0.0f, 1.0f);
    }

    static Color green()
    {
        return Color(0.0f, 1.0f, 0.0f, 1.0f);
    }

    static Color blue()
    {
        return Color(0.0f, 0.0f, 1.0f, 1.0f);
    }

    static Color white()
    {
        return Color(1.0f, 1.0f, 1.0f, 1.0f);
    }

    static Color black()
    {
        return Color(0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Color yellow()
    {
        return Color(1.0, 1.0f, 0.0f, 1.0f);
    }

    static Color cyan()
    {
        return Color(0.0, 1.0f, 1.0f, 1.0f);
    }

    static Color magenta()
    {
        return Color(1.0, 0.0f, 1.0f, 1.0f);
    }

    static Color darkRed()
    {
        return Color(0.2f, 0.0f, 0.0f, 1.0f);
    }
};

inline float lerp(float t, float a, float b)
{
    return (1.0f - t)*a + t*b;
}

inline Color lerpColor(float t, const Color &a, const Color b)
{
    return Color(lerp(t, a.r, b.r), lerp(t, a.g, b.g), lerp(t, a.b, b.b), lerp(t, a.a, b.a));
}

}

#endif //_TAREA2_COLOR_HPP
