#include <stdlib.h>
#include "HardwareBuffer.hpp"

namespace Tarea2
{

HardwareBuffer::HardwareBuffer()
{
    capacity = 0;
    elementSize = 0;
    invalid = true;
    usage = GL_STATIC_DRAW;
    glGenBuffers(1, &handle);
}

HardwareBuffer::~HardwareBuffer()
{
    glDeleteBuffers(1, &handle);
}

GLuint HardwareBuffer::getHandle()
{
    return handle;
}

GLenum HardwareBuffer::getUsage() const
{
    return this->usage;
}

void HardwareBuffer::setUsage(GLenum usage)
{
    this->usage = usage;
    invalid = true;
}

size_t HardwareBuffer::getElementSize() const
{
    return elementSize;
}

void HardwareBuffer::setElementSize(size_t elementSize)
{
    this->elementSize = elementSize;
}

size_t HardwareBuffer::getCapacity() const
{
    return this->capacity;
}

void HardwareBuffer::setCapacity(size_t capacity)
{
    this->capacity = capacity;
    invalid = true;
}

void HardwareBuffer::upload(size_t offset, size_t len, void *data)
{
    // Create the buffer object.
    createObject();

    // Send the data.
    glBindBuffer(getTarget(), handle);
    glBufferSubData(getTarget(), offset, len, data);
}

void HardwareBuffer::createObject()
{
    if(!invalid)
        return;

    glBindBuffer(getTarget(), handle);
    glBufferData(getTarget(), capacity, NULL, usage);
    invalid = false;
}

void HardwareBuffer::activate()
{
    glBindBuffer(getTarget(), handle);
}

//-----------------------------------------------------------------------------
const VertexDescription *VertexBuffer::getVertexDescription() const
{
    return this->vertexDescription;
}

void VertexBuffer::setVertexDescription(const VertexDescription *vertexDescription)
{
    this->vertexDescription = vertexDescription;
}

void VertexBuffer::activate()
{
    HardwareBuffer::activate();

    // Enable the vertex attributes.
    enableVertexAttributes();
}

void VertexBuffer::enableVertexAttributes()
{
    // TODO: Disable all attributes.
    size_t vertexSize = vertexDescription->size;
    for(size_t i = 0; i < vertexDescription->numattributes; ++i)
    {
        const VertexAttributeDescription &attribute = vertexDescription->attributes[i];
        glEnableVertexAttribArray(attribute.binding);
        glVertexAttribPointer(attribute.binding, attribute.numcomponents, attribute.type, GL_FALSE, vertexSize, reinterpret_cast<void*> (attribute.offset));
    }
}

//-----------------------------------------------------------------------------
GLenum IndexBuffer::getElementType()
{
    switch(getElementSize())
    {
    case 1:
        return GL_UNSIGNED_BYTE;
    case 2:
        return GL_UNSIGNED_SHORT;
    case 4:
        return GL_UNSIGNED_INT;
    default:
        abort();
    }
}

void IndexBuffer::drawElements(GLenum mode, size_t count, size_t offset)
{
    glDrawElements(mode, count, getElementType(), reinterpret_cast<void*> (offset));
}

}

