#ifndef _TAREA2_HARDWARE_BUFFER_HPP
#define _TAREA2_HARDWARE_BUFFER_HPP

#include <GL/glew.h>

namespace Tarea2
{

/**
 * Vertex attribute description.
 */
struct VertexAttributeDescription
{
    GLuint binding;
    size_t offset;
    size_t numcomponents;
    GLenum type;
};

/**
 * Vertex description
 */
struct VertexDescription
{
    size_t size;
    size_t numattributes;
    VertexAttributeDescription *attributes;
};

/**
 * GPU hardware buffer.
 */
class HardwareBuffer
{
public:
    HardwareBuffer();
    virtual ~HardwareBuffer();

    virtual GLenum getTarget() = 0;
    GLuint getHandle();

    GLenum getUsage() const;
    void setUsage(GLenum usage);

    size_t getElementSize() const;
    void setElementSize(size_t elementSize);

    size_t getCapacity() const;
    void setCapacity(size_t capacity);

    void upload(size_t offset, size_t len, void *data);

    virtual void activate();

private:
    void createObject();

    GLuint handle;
    GLenum usage;
    size_t capacity;
    size_t elementSize;
    bool invalid;
};

/**
 * Vertex buffer object.
 */
class VertexBuffer: public HardwareBuffer
{
public:
    const VertexDescription *getVertexDescription() const;
    void setVertexDescription(const VertexDescription *vertexDescription);

    virtual GLenum getTarget()
    {
        return GL_ARRAY_BUFFER;
    }

    virtual void activate();

private:
    void enableVertexAttributes();

    const VertexDescription *vertexDescription;
};

/**
 * Index buffer object.
 */
class IndexBuffer: public HardwareBuffer
{
public:
    virtual GLenum getTarget()
    {
        return GL_ELEMENT_ARRAY_BUFFER;
    }

    void drawElements(GLenum mode, size_t count, size_t offset);

private:
    GLenum getElementType();
};

} // namespace Tarea2

#endif //_TAREA2_HARDWARE_BUFFER_HPP

