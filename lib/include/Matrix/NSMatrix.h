#pragma once

#include <CSRMatrix.h>
#include <Mesh.h>

class NSMatrix : public CSRMatrix
{
public:
    Mesh &mesh;

    NSMatrix(Mesh &m) : CSRMatrix(m), mesh(m) {}
};