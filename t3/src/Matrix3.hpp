#ifndef _T3_MATRIX3_HPP
#define _T3_MATRIX3_HPP

#include "Vector2.hpp"
#include "Vector3.hpp"

namespace T3
{

/**
 * 3x3 Matrix
 */
class Matrix3
{
public:
    explicit Matrix3() {}
    Matrix3(float newData[9])
    {
        for(int i = 0; i < 9; ++i)
            this->data[i] = newData[i];
    }

    float at(int i) const
    {
        return data[i];
    }

    float at(int i, int j) const
    {
        return data[i*3 + j];
    }

    void setAt(int i, float v)
    {
        data[i] = v;
    }

    void setAt(int i, int j, float v)
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

    Matrix3 transpose() const
    {
        Matrix3 ret;
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                ret.setAt(i, j, at(j, i));
        return ret;
    }

public:
    friend Matrix3 operator*(const Matrix3 &a, const Matrix3 &b)
    {
        Matrix3 ret;
        for(int i = 0; i < 3; ++i)
        {
            for(int j = 0; j < 3; ++j)
            {
                float s = 0.0f;
                for(int k = 0; k < 3; ++k)
                    s += a.at(i, k)*b.at(k, j);
                ret.setAt(i, j, s);
            }
        }

        return ret;
    }

    friend Vector3 operator*(const Matrix3 &a, const Vector3 &v)
    {
        return Vector3(a.at(0, 0)*v.x + a.at(0, 1)*v.y + a.at(0, 2)*v.z,
                       a.at(1, 0)*v.x + a.at(1, 1)*v.y + a.at(1, 2)*v.z,
                       a.at(2, 0)*v.x + a.at(2, 1)*v.y + a.at(2, 2)*v.z);
    }

    void dump() const
    {
        for(int i = 0; i < 3; ++i)
            printf("%f %f %f\n", at(i, 0), at(i, 1), at(i, 2));
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

    static Matrix3 xRot(const float angle)
    {
        float c = cos(angle);
        float s = sin(angle);
        float data[9] = {
            1, 0, 0,
            0, c, -s,
            0, s, c,
        };
        return Matrix3(data);
    }

    static Matrix3 yRot(const float angle)
    {
        float c = cos(angle);
        float s = sin(angle);
        float data[9] = {
            c, 0, s,
            0, 1, 0,
            -s, 0, c
        };
        return Matrix3(data);
    }

    static Matrix3 zRot(const float angle)
    {
        float c = cos(angle);
        float s = sin(angle);
        float data[9] = {
            c, -s, 0,
            s, c, 0,
            0, 0, 1
        };
        return Matrix3(data);
    }

    static Matrix3 xyzRot(const Vector3 &rotation)
    {
        return xRot(rotation.x)*yRot(rotation.y)*zRot(rotation.z);
    }

private:
    float data[3*3];
};

}

#endif //_T3_MATRIX3_HPP

