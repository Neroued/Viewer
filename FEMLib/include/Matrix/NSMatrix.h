#pragma once

#include <NameSpace.h>
#include <CSRMatrix.h>
#include <Mesh.h>

NAMESPACE_BEGIN(FEMLib)

class NSMatrix : public CSRMatrix
{
public:
    Mesh &mesh;

    NSMatrix(Mesh &m) : CSRMatrix(m), mesh(m) {}
};

NAMESPACE_END