#ifndef _TAREA2_MESH_BUILDER_HPP
#define _TAREA2_MESH_BUILDER_HPP

#include "Vector2.hpp"
#include "Color.hpp"
#include "Mesh.hpp"

namespace Tarea2
{
/**
 * Mesh builder.
 */
template<typename VT, typename IT>
class MeshBuilder
{
public:
    typedef VT VertexType;
    typedef IT IndexType;
    typedef Mesh<VT,IT> MeshType;

    MeshBuilder()
    {
        mesh = new MeshType();
        currentTransform = Matrix3::identity();
        baseVertex = 0;
        baseIndex = 0;
    }

    ~MeshBuilder()
    {
        delete mesh;
    }

    bool empty() const
    {
        return mesh->empty();
    }

    void addVertex(const Vector2 &position, const Color &color, const Vector2 &normal)
    {
        mesh->addVertex(VertexType(currentTransform.transformPosition(position), color, currentTransform.transformDirection(normal)));
    }

    void addIndex(IT index)
    {
        mesh->addIndex(index + baseVertex);
        indexCount++;
    }

    void endTriangles()
    {
        // Try to add into the last submesh.
        SubMesh *last = mesh->getLastSubMesh();
        if(last != NULL && last->getMode() == GL_TRIANGLES)
            last->setIndexCount(last->getIndexCount() + indexCount);
        else
            mesh->addSubMesh(GL_TRIANGLES, baseIndex, indexCount);

        // Create a new geometry base.
        newGeometryBase();
    }

    void endLines()
    {
        // Try to add into the last submesh.
        SubMesh *last = mesh->getLastSubMesh();
        if(last != NULL && last->getMode() == GL_LINES)
            last->setIndexCount(last->getIndexCount() + indexCount);
        else
            mesh->addSubMesh(GL_LINES, baseIndex, indexCount);

        // Create a new geometry base.
        newGeometryBase();
    }

    void clear()
    {
        mesh->clear();
        baseVertex = 0;
        baseIndex = 0;
        indexCount = 0;
    }

    MeshType *getMesh()
    {
        return mesh;
    }

    void setTransform(const Matrix3 &transform)
    {
        this->currentTransform = transform;
    }

    const Matrix3 &getTransform() const
    {
        return currentTransform;
    }

private:
    void newGeometryBase()
    {
        baseVertex = mesh->getVertexCount();
        baseIndex = mesh->getIndexCount();
        indexCount = 0;
    }

    MeshType *mesh;
    Matrix3 currentTransform;

    IndexType baseVertex;
    size_t baseIndex;
    size_t indexCount;
};

typedef MeshBuilder<SimpleVertex, int> SimpleMeshBuilder;

}

#endif //_TAREA2_MESH_BUILDER_HPP
