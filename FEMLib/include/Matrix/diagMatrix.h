#pragma once

#include <NameSpace.h>
#include <Matrix.h>
#include <TArray.h>

NAMESPACE_BEGIN(FEMLib)

class diagMatrix : public Matrix
{
public:
    Vec diag;

    diagMatrix(int r);

    void MVP(const Vec &x, Vec &y) const;
    void MVP_inverse(const Vec &x, Vec &y) const;
};

NAMESPACE_END