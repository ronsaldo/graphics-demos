#ifndef _T3_COLOR_HPP
#define _T3_COLOR_HPP

#include <CL/cl.h>
#include "Vector2.hpp" // For closeTo

namespace T3
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

    union
    {
        struct
        {
            float r, g, b, a;
        };
        cl_float4 data;
    };

public:
    Color applyIntensity(float intensity) const
    {
        return Color(r*intensity, g*intensity, b*intensity, a);
    }

public:
    bool closeTo(const Color &c) const
    {
        return T3::closeTo(r, c.r) && T3::closeTo(g, c.g) &&
               T3::closeTo(b, c.b) && T3::closeTo(a, c.a);
    }

public:
    friend Color operator+(const Color &a, const Color &b)
    {
        return Color(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);
    }

    friend Color operator*(float s, const Color c)
    {
        return Color(c.r*s, c.g*s, c.b*s, c.a*s);
    }

    friend Color operator*(const Color c, const float s)
    {
        return Color(c.r*s, c.g*s, c.b*s, c.a*s);
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

inline Color color_black()
{
    return Color::black();
}

inline Color color_white()
{
    return Color::white();
}

inline float lerp(float t, float a, float b)
{
    return (1.0f - t)*a + t*b;
}

inline Color lerpColor(float t, const Color &a, const Color b)
{
    return Color(lerp(t, a.r, b.r), lerp(t, a.g, b.g), lerp(t, a.b, b.b), lerp(t, a.a, b.a));
}

}

#endif //_T3_COLOR_HPP
