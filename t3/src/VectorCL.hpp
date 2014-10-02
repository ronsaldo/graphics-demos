#ifndef T3_VECTOR_CL_HPP
#define T3_VECTOR_CL_HPP

#define constant_vector2(x, y) (float2)(x, y)
#define constant_vector3(x, y, z) (float3)(x, y, z)
#define constant_vector4(x, y, z, w) (float4)(x, y, z, w)


typedef float2 Vector2;
typedef float3 Vector3;
typedef float4 Vector4;
typedef float4 Color;

inline float2 make_vector2(float x, float y)
{
    return (float2)(x, y);
}

inline float3 make_vector3(float x, float y, float z)
{
    return (float3)(x, y, z);
}

inline float4 make_vector4(float x, float y, float z, float w)
{
    return (float4)(x, y, z, w);
}

inline float2 vector2_zero()
{
    return make_vector2(0.0f, 0.0f);
}

inline float3 vector3_zero()
{
    return make_vector3(0.0f, 0.0f, 0.0f);
}

inline float4 vector4_zero()
{
    return make_vector4(0.0f, 0.0f, 0.0f, 0.0f);
}

inline Color make_color(float r, float g, float b, float a)
{
    return make_vector4(r, g, b, a);
}

inline Color color_white()
{
    return make_color(1.0f, 1.0f, 1.0f, 1.0f);
}

inline Color color_black()
{
    return make_color(0.0f, 0.0f, 0.0f, 1.0f);
}

inline Color color_zero()
{
    return make_color(0.0f, 0.0f, 0.0f, 0.0f);
}

#endif //T3_VECTOR_CL_HPP
