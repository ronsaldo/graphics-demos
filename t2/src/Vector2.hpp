#ifndef _TAREA2_VECTOR2_HPP
#define _TAREA2_VECTOR2_HPP

#include <math.h>

namespace Tarea2
{
const float FloatEpsilon = 0.00001f;

/**
 * Tells if two floats are close.
 */
inline bool closeTo(float a, float b)
{
    float dif = a - b;
    return -FloatEpsilon <= dif && dif <= FloatEpsilon;
}

/**
 * 2D vector.
 */
class Vector2
{
public:
    explicit Vector2(float x = 0, float y = 0)
        : x(x), y(y) {}
    ~Vector2() {}

    float x, y;

public:
    friend Vector2 operator+(const Vector2 &a, const Vector2 &b)
    {
        return Vector2(a.x + b.x, a.y + b.y);
    }

    friend Vector2 operator-(const Vector2 &a, const Vector2 &b)
    {
        return Vector2(a.x - b.x, a.y - b.y);
    }

    friend Vector2 operator*(const Vector2 &v, float s)
    {
        return Vector2(v.x*s, v.y*s);
    }

    friend Vector2 operator*(float s, const Vector2 &v)
    {
        return Vector2(v.x*s, v.y*s);
    }

    friend Vector2 operator/(const Vector2 &v, float s)
    {
        return v*(1.0/s);
    }

    float dot(const Vector2 &v) const
    {
        return x*v.x + y*v.y;
    }

    float length2() const
    {
        return x*x + y*y;
    }

    float length() const
    {
        return sqrt(x*x + y*y);
    }

    Vector2 normalized() const
    {
        return (*this)/length();
    }

    bool closeTo(const Vector2 &v) const
    {
        return Tarea2::closeTo(x, v.x) && Tarea2::closeTo(y, v.y);
    }

public:
    static Vector2 zero()
    {
        return Vector2(0, 0);
    }
};

} // namespace Tarea2

#endif //_TAREA2_VECTOR2_HPP
