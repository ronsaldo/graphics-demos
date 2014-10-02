#ifndef _TAREA2_VECTOR3_HPP_
#define _TAREA2_VECTOR3_HPP_

namespace Tarea2
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

    float x, y, z;
};

}

#endif //_TAREA2_VECTOR3_HPP_

