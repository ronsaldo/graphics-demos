#include <stddef.h>
#include "SimpleVertex.hpp"

namespace Tarea2
{

static VertexAttributeDescription simpleVertexAttributes[] = {
    {1, offsetof(SimpleVertex, position), 2, GL_FLOAT},
    {2, offsetof(SimpleVertex, color), 4, GL_FLOAT},
    {3, offsetof(SimpleVertex, normal), 2, GL_FLOAT}
};

static VertexDescription SimpleVertexDescription = {
    sizeof(SimpleVertex),
    3,
    simpleVertexAttributes,
};

const VertexDescription *SimpleVertex::getDescription()
{
    return &SimpleVertexDescription;
}

}

