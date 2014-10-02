#ifndef _TAREA2_SIMPLE_VERTEX_HPP
#define _TAREA2_SIMPLE_VERTEX_HPP

#include "Vector2.hpp"
#include "Color.hpp"
#include "HardwareBuffer.hpp"

namespace Tarea2
{

/**
 * Simple vertex.
 */
struct SimpleVertex
{
    SimpleVertex() {}
    SimpleVertex(const Vector2 &position, const Color &color, const Vector2 &normal)
        : position(position), color(color), normal(normal) {}
    ~SimpleVertex() {}

    Vector2 position;
    Color color;
    Vector2 normal;

    static const VertexDescription *getDescription();
};

}

#endif //_TAREA2_SIMPLE_VERTEX_HPP
