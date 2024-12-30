#pragma once

#include <Matrix.h>
#include <TArray.h>
#include <stdexcept>

class normalMatrix : public Matrix
{
public:
    double *data;

    normalMatrix(int r, int c) : Matrix(r, c) { data = new double[r * c]; }
    ~normalMatrix() = default;

    void MVP(const Vec &x, Vec &y);
};

void normalMatrix::MVP(const Vec &x, Vec &y)
{
    if (rows != x.size || rows != y.size)
    {
        throw std::invalid_argument("Size mismatch: The number of columns in the matrix does not match the size of the vector.");
    }

    y.setAll(0);

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < rows; ++j)
        {
            y[i] += data[i * cols + j] * x[j];
        }
    }
}