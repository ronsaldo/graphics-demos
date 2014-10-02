#ifndef _TAREA2_MATRIX3_HPP
#define _TAREA2_MATRIX3_HPP

#include "Vector2.hpp"

namespace Tarea2
{

/**
 * 3x3 Matrix
 */
class Matrix3
{
public:
    explicit Matrix3() {}
    Matrix3(float data[9])
    {
        for(int i = 0; i < 9; ++i)
            this->data[i] = data[i];
    }

    float at(int i) const
    {
        return data[i];
    }

    float at(int i, int j) const
    {
        return data[i*3 + j];
    }

    void setAt(int i, int v)
    {
        data[i] = v;
    }

    void setAt(int i, int j, int v)
    {
        data[i*3 + j] = v;
    }

    float *rawData()
    {
        return data;
    }

    const float *rawData() const
    {
        return data;
    }

public:
    Vector2 transformPosition(const Vector2 &v)
    {
        return Vector2(at(0,0)*v.x + at(0, 1)*v.y + at(0, 2),
                       at(1,0)*v.x + at(1, 1)*v.y + at(1, 2));
    }

    Vector2 transformDirection(const Vector2 &v)
    {
        return Vector2(at(0,0)*v.x + at(0, 1)*v.y,
                       at(1,0)*v.x + at(1, 1)*v.y);
    }

public:
    static Matrix3 zero()
    {
        float data[9]  = {
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
        };
        return Matrix3(data);
    }

    static Matrix3 ones()
    {
        float data[9]  = {
            1, 1, 1,
            1, 1, 1,
            1, 1, 1,
        };
        return Matrix3(data);
    }

    static Matrix3 identity()
    {
        float data[9]  = {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
        };
        return Matrix3(data);
    }

    static Matrix3 translation(const Vector2 &t)
    {
        float data[9] = {
            1, 0, t.x,
            0, 1, t.y,
            0, 0, 1
        };
        return Matrix3(data);
    }

private:
    float data[3*3];
};

}

#endif //_TAREA2_MATRIX3_HPP

