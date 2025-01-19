#pragma once

#include <NameSpace.h>
#include <TArray.h>
#include <vec3.h>
#include <cstdint>

NAMESPACE_BEGIN(FEMLib)

typedef int MeshType;

class Mesh
{
public:
    TArray<Vec3> vertices;    // 存储(x,y,z)点的坐标
    TArray<uint32_t> indices; // 每三个顶点为一组表示三角形
    MeshType meshtype;
    int subdiv;
    int *dupToNoDupIndex;

    size_t vertex_count() const { return vertices.size; }
    size_t triangle_count() const { return indices.size / 3; }

    Mesh() = default;
    Mesh(int subdiv, MeshType meshtype);
    Mesh(int subdiv, MeshType meshtype, bool saveDTND);

   ~Mesh(); 
};

#ifndef CUBE
#define CUBE 1
#endif

#ifndef SPHERE
#define SPHERE 2
#endif

int load_cube(Mesh &m, const int subdiv);
int load_sphere(Mesh &m, const int subdiv);
int load_cube(Mesh &m, const int subdiv, bool saveDTND);
int load_sphere(Mesh &m, const int subdiv, bool saveDTND);

NAMESPACE_END