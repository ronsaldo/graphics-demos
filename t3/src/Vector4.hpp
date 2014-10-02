#ifndef T3_VECTOR4_HPP
#define T3_VECTOR4_HPP

#include <CL/cl.h>
#include "Vector3.hpp"

namespace T3
{

/**
 * 4D vector.
 */
class Vector4
{
public:
    Vector4(const Vector3 &v, float w = 0.0f)
        : x(v.x), y(v.y), z(v.z), w(w) {}
    explicit Vector4(float x=0.0f, float y=0.0f, float z=0.0f, float w=0.0f)
        : x(x), y(y), z(z), w(w) {}
    ~Vector4() {}

    union
    {
        struct
        {
            float x, y, z, w;
        };
        cl_float4 data;
    };

    float dot(const Vector4 &v) const
    {
        return x*v.x + y*v.y + z*v.z + w*v.w;
    }

};

inline Vector4 make_vector4(float x, float y, float z, float w)
{
    return Vector4(x, y, z, w);
}

}

#endif //T3_VECTOR4_HPP

