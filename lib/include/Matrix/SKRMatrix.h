#pragma once

#include <Matrix.h>
#include <TArray.h>
#include <cstdint>
#include <CSRMatrix.h>

/* Skyline Sparse Symmetric Matrix*/
class SKRMatrix : public Matrix
{
public:
    Vec elements;
    TArray<size_t> column_offset;

    SKRMatrix() = default;
    SKRMatrix(int r) : Matrix(r, r) {}
    SKRMatrix(CSRMatrix &A); // Initialize from CSRMatrix for Cholesky

    void MVP(const Vec &x, Vec &y) const;
    void convertFromCSR(const CSRMatrix &A); // Convert a CSRMatrix to SKRMatrix
    void print() const;
};