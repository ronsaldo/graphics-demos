#ifndef _T3_VECTOR3_HPP_
#define _T3_VECTOR3_HPP_

#include <CL/cl.h>
#include "Vector2.hpp"

namespace T3
{

/**
 * 3D vector.
 */
class Vector3
{
public:
    explicit Vector3(float x=0.0f, float y=0.0f, float z=0.0f)
        : x(x), y(y), z(z) {}
    ~Vector3() {}

    union
    {
        struct
        {
            float x, y, z;
        };
        cl_float3 data;
    };


public:
    friend Vector3 operator+(const Vector3 &a, const Vector3 &b)
    {
        return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    friend Vector3 operator-(const Vector3 &v)
    {
        return Vector3(-v.x, -v.y, -v.z);
    }

    friend Vector3 operator-(const Vector3 &a, const Vector3 &b)
    {
        return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    friend Vector3 operator*(const Vector3 &a, const Vector3 &b)
    {
        return Vector3(a.x*b.x, a.y*b.y, a.z*b.z);
    }

    friend Vector3 operator*(const Vector3 &a, const float s)
    {
        return Vector3(a.x*s, a.y*s, a.z*s);
    }

    friend Vector3 operator*(const float s, const Vector3 &a)
    {
        return Vector3(a.x*s, a.y*s, a.z*s);
    }

    friend Vector3 operator/(const Vector3 &a, const float s)
    {
        return a*(1.0f/s);
    }

    friend bool operator==(const Vector3 &a, const Vector3 &b)
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    friend bool operator!=(const Vector3 &a, const Vector3 &b)
    {
        return !(a == b);
    }

    Vector3 &operator+=(const Vector3 &b)
    {
        x += b.x;
        y += b.y;
        z += b.z;
        return *this;
    }

    float dot(const Vector3 &v) const
    {
        return x*v.x + y*v.y + z*v.z;
    }

    float length2() const
    {
        return x*x + y*y + z*z;
    }

    float length() const
    {
        return sqrt(x*x + y*y + z*z);
    }

    Vector3 normalized() const
    {
        return (*this)/length();
    }

public:
    static Vector3 zero()
    {
        return Vector3(0, 0, 0);
    }
};

inline Vector3 make_vector3(float x, float y, float z)
{
    return Vector3(x, y, z);
}

inline Vector3 cross(const Vector3 &a, const Vector3 &b)
{
    abort();
}

}

#endif //_T3_VECTOR3_HPP_

