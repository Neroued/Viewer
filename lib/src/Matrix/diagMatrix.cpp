#include <diagMatrix.h>
#include <TArray.h>

diagMatrix::diagMatrix(int r)
    : Matrix(r, r), diag(r)
{
}

void diagMatrix::MVP(const Vec &x, Vec &y) const
{
    if (cols != x.size || cols != y.size)
    {
        throw std::invalid_argument("Size mismatch: The number of columns in the matrix does not match the size of the vector.");
    }

    for (size_t i = 0; i < x.size; ++i)
    {
        y[i] = diag[i] * x[i];
    }
}

void diagMatrix::MVP_inverse(const Vec &x, Vec &y) const
{
    if (cols != x.size || cols != y.size)
    {
        throw std::invalid_argument("Size mismatch: The number of columns in the matrix does not match the size of the vector.");
    }

    for (size_t i = 0; i < x.size; ++i)
    {
        y[i] = x[i] / diag[i];
    }
}
