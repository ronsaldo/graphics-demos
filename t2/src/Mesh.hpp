#ifndef _TAREA2_MESH_HPP
#define _TAREA2_MESH_HPP

#include <vector>

#include "SimpleVertex.hpp"
#include "HardwareBuffer.hpp"

namespace Tarea2
{

/**
 * Mesh base.
 */
class BaseMesh
{
public:
    virtual ~BaseMesh() {}

    virtual IndexBuffer *getIndexBuffer() = 0;
    virtual VertexBuffer *getVertexBuffer() = 0;
};

/**
 * SubMesh
 */
class SubMesh
{
public:
    SubMesh();
    ~SubMesh();

    size_t getFirstIndex() const;
    size_t getIndexCount() const;
    GLenum getMode() const;

    void setFirstIndex(size_t firstIndex);
    void setIndexCount(size_t indexCount);
    void setMode(GLenum mode);

    void draw(BaseMesh *mesh);

private:
    size_t firstIndex;
    size_t indexCount;
    GLenum mode;
};

/**
 * Mesh
 */
template<typename VT, typename IT>
class Mesh: public BaseMesh
{
public:
    typedef VT VertexType;
    typedef IT IndexType;

    Mesh()
    {
        indexBuffer = NULL;
        vertexBuffer = NULL;
        invalid = true;
    }

    ~Mesh()
    {
        clear();
        delete indexBuffer;
        delete vertexBuffer;
    }

    SubMesh *addSubMesh()
    {
        SubMesh *res = new SubMesh();
        submeshes.push_back(res);
        return res;
    }

    SubMesh *addSubMesh(GLenum mode, size_t first, size_t count)
    {
        SubMesh *res = addSubMesh();
        res->setMode(mode);
        res->setFirstIndex(first);
        res->setIndexCount(count);
        return res;
    }

    bool empty() const
    {
        return indices.empty();
    }

    void addVertex(const VertexType &vertex)
    {
        vertices.push_back(vertex);
        invalid = true;
    }

    void addIndex(IndexType index)
    {
        indices.push_back(index);
        invalid = true;
    }

    void clear()
    {
        for(size_t i = 0; i < submeshes.size(); ++i)
            delete submeshes[i];
        submeshes.clear();
        vertices.clear();
        indices.clear();
    }

    void draw()
    {
        // Validate.
        validate();

        // Activate the buffers.
        vertexBuffer->activate();
        indexBuffer->activate();

        // Draw the submeshes.
        for(size_t i = 0; i < submeshes.size(); ++i)
            submeshes[i]->draw(this);
    }

    virtual IndexBuffer *getIndexBuffer()
    {
        return this->indexBuffer;
    }

    virtual VertexBuffer *getVertexBuffer()
    {
        return this->vertexBuffer;
    }

    size_t getIndexCount() const
    {
        return indices.size();
    }

    size_t getVertexCount() const
    {
        return vertices.size();
    }

    SubMesh *getLastSubMesh()
    {
        if(submeshes.empty())
            return NULL;
        return submeshes.back();
    }

private:
    void createBuffers()
    {
        // Create the vertex buffer
        vertexBuffer = new VertexBuffer();
        vertexBuffer->setVertexDescription(VertexType::getDescription());
        vertexBuffer->setElementSize(sizeof(VertexType));

        // Create the index buffer.
        indexBuffer = new IndexBuffer();
        indexBuffer->setElementSize(sizeof(IndexType));
    }

    void validate()
    {
        if(!invalid)
            return;
        if(!vertexBuffer || !indexBuffer)
            createBuffers();

        sendVertices();
        sendIndices();
        invalid = false;
    }

    void sendVertices()
    {
        size_t length = vertices.size()*sizeof(VertexType);
        if(vertexBuffer->getCapacity() < length)
            vertexBuffer->setCapacity(length);
        vertexBuffer->upload(0, length, &vertices[0]);
    }

    void sendIndices()
    {
        size_t length = indices.size()*sizeof(IndexType);
        if(indexBuffer->getCapacity() < length)
            indexBuffer->setCapacity(length);
        indexBuffer->upload(0, length, &indices[0]);
    }

    std::vector<SubMesh*> submeshes;
    std::vector<VertexType> vertices;
    std::vector<IndexType> indices;
    IndexBuffer *indexBuffer;
    VertexBuffer *vertexBuffer;
    bool invalid;
};

typedef Mesh<SimpleVertex, int> SimpleMesh;
} // namespace Tarea2

#endif //_TAREA2_MESH_HPP
