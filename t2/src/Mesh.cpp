#include "Mesh.hpp"

namespace Tarea2
{
SubMesh::SubMesh()
{
    firstIndex = 0;
    indexCount = 0;
    mode = GL_POINTS;
}

SubMesh::~SubMesh()
{
}

size_t SubMesh::getFirstIndex() const
{
    return this->firstIndex;
}

size_t SubMesh::getIndexCount() const
{
    return this->indexCount;
}

GLenum SubMesh::getMode() const
{
    return this->mode;
}

void SubMesh::setFirstIndex(size_t firstIndex)
{
    this->firstIndex = firstIndex;
}

void SubMesh::setIndexCount(size_t indexCount)
{
    this->indexCount = indexCount;
}

void SubMesh::setMode(GLenum mode)
{
    this->mode = mode;
}

void SubMesh::draw(BaseMesh *mesh)
{
    mesh->getIndexBuffer()->drawElements(mode, indexCount, firstIndex);
}

} // namespace Tarea2
