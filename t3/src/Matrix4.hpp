#ifndef _T3_MATRIX4_HPP
#define _T3_MATRIX4_HPP

#include "Matrix3.hpp"

namespace T3
{

/**
 * 4x4 Matrix
 */
class Matrix4
{
public:
    explicit Matrix4() {}
    Matrix4(float data[16])
    {
        for(int i = 0; i < 16; ++i)
            this->data[i] = data[i];
    }

    float at(int i) const
    {
        return data[i];
    }

    float at(int i, int j) const
    {
        return data[i*4 + j];
    }

    void setAt(int i, int v)
    {
        data[i] = v;
    }

    void setAt(int i, int j, int v)
    {
        data[i*4 + j] = v;
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
    static Matrix4 zeros()
    {
        float data[16]  = {
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        };
        return Matrix4(data);
    }

    static Matrix4 ones()
    {
        float data[16]  = {
            1, 1, 1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
        };
        return Matrix4(data);
    }

    static Matrix4 identity()
    {
        float data[16]  = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
        };
        return Matrix4(data);
    }

    static Matrix4 translation(const Vector2 &t)
    {
        float data[16] = {
            1, 0, 0, t.x,
            0, 1, 0, t.y,
            0, 0, 1, 0,
            0, 0, 0, 1,
        };
        return Matrix4(data);
    }

    static Matrix4 from2DTransform(const Matrix3 &t)
    {
        float data[16] = {
            t.at(0, 0), t.at(0, 1), 0, t.at(0, 2),
            t.at(1, 0), t.at(1, 1), 0, t.at(1, 2),
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        return Matrix4(data);
    }

    static Matrix4 ortho(float left, float right, float bottom, float top, float near, float far)
    {
        // Specification from: http://www.opengl.org/sdk/docs/man2/xhtml/glOrtho.xml

        float tx = -(right + left)/(right - left);
        float ty = -(top + bottom)/(top - bottom);
        float tz = -(far + near)/(far - near);

        float data[16] = {
            2.0f/(right - left), 0, 0, tx,
            0, 2.0f/(top - bottom), 0, ty,
            0, 0, -2.0f/(far - near), tz,
            0, 0, 0, 1,
        };
        return Matrix4(data);
    }

private:
    float data[4*4];
};

}

#endif //_TAREA2_MATRIX4_HPP

