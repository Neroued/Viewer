#pragma once

#include <NameSpace.h>
#include <TArray.h>
#include <cstdint>

NAMESPACE_BEGIN(FEMLib)

using VertexCoord = float;
using VertexIndex = uint32_t; // or size_t
using TriangleIndex = VertexIndex;
using BoundaryIndex = TriangleIndex;

enum class MeshType
{
    CUBE,
    SPHERE,
    SQUARE,
    HEMI_SPHERE
};

class Mesh
{
public:
    TArray<VertexCoord> m_vertices;          // 每三个点表示一个坐标
    TArray<TriangleIndex> m_triangleIndices; // 每三个顶点下标为一组表示三角形
    TArray<BoundaryIndex> m_boundaryIndices; // 每两个顶点下标表示一条边
    MeshType m_meshtype;

    size_t vertex_count() const { return m_vertices.size / 3; }
    size_t triangle_count() const { return m_triangleIndices.size / 3; }
    size_t boundary_count() const { return m_boundaryIndices.size / 2; }

    VertexCoord *vertex(const VertexIndex i) { return m_vertices.data + i * 3; }              // 返回第i个顶点的第一个分量对应的指针
    TriangleIndex *triangle(const TriangleIndex i) { return m_triangleIndices.data + i * 3; } // 返回第i个三角形的第一个分量对应的指针
    BoundaryIndex *boundary(const BoundaryIndex i) { return m_boundaryIndices.data + i * 2; } // 返回第i个边界的第一个分量对应的指针

    Mesh() = default;
    Mesh(int subdiv, MeshType meshtype);

    ~Mesh();
};

int load_cube(Mesh &m, const int subdiv);
int load_sphere(Mesh &m, const int subdiv);
int load_square(Mesh &m, const int subdiv);
int load_hemisphere(Mesh &m, const int subdiv);
void deduplicateVertices(Mesh &mesh);

NAMESPACE_END