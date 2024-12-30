#pragma once

#include <Matrix.h>
#include <TArray.h>

class diagMatrix : public Matrix
{
public:
    Vec diag;

    diagMatrix(int r);

    void MVP(const Vec &x, Vec &y) const;
    void MVP_inverse(const Vec &x, Vec &y) const;
};