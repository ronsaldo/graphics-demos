#ifndef T3_COMMONCL_HPP
#define T3_COMMONCL_HPP

#ifdef CL_RAYTRACER
#define NULL 0
#include "VectorCL.hpp"

#else
#include <stdlib.h>
#include <stdio.h>
#include "Color.hpp"
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
using namespace T3;

#define __private
#define __constant
#define __global
#define __local

#define constant_vector2 Vector2
#define constant_vector3 Vector3
#define constant_vector4 Vector4

template<typename T>
T mix(T a, T b, float alpha)
{
    return (1.0f - alpha)*a + alpha*b;
}

#define get_global_id(x) 0

#endif


static const __constant Vector4 vector_axis[] = {
    constant_vector4(1, 0, 0, 0),
    constant_vector4(0, 1, 0, 0),
    constant_vector4(0, 0, 1, 0),
    constant_vector4(1, 0, 0, 0),
    constant_vector4(0, 1, 0, 0),
    constant_vector4(0, 0, 1, 0),
};

inline Vector3 vectorAxis(Vector4 v)
{
#ifdef CL_RAYTRACER
    return v.xyz;
#else
    abort();
#endif
}

inline int bestVectorAxis(Vector3 v)
{
    float bestVal = fabs(dot(v, vectorAxis(vector_axis[0])));
    int best = 0;
    float val = fabs(dot(v, vectorAxis(vector_axis[1])));
    if(val > bestVal)
        best = 1;
    val = fabs(dot(v, vectorAxis(vector_axis[2])));
    if(val > bestVal)
        best = 2;
    return best;
}

#endif //T3_COMMONCL_HPP
