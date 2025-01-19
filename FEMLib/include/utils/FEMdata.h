#pragma once

#include <NameSpace.h>
#include <TArray.h>
#include <NSMatrix.h>
#include <Mesh.h>
#include <vec3.h>

NAMESPACE_BEGIN(FEMLib)

class FEMData
// 存储求解-\Delta u + u = f的相关结果
{
public:
    Mesh mesh;
    NSMatrix A;
    Vec u;
    Vec B;

    FEMData(int subdiv, MeshType meshtype, double (*func)(Vec3 pos));
};

NAMESPACE_END
