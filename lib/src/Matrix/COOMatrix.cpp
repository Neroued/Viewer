#include <COOMatrix.h>

void COOMatrix::MVP(const Vec &x, Vec &y) const
{
    // 初始化out
    y.setAll(0);
    if (cols != x.size || cols != y.size)
    {
        throw std::invalid_argument("Size mismatch: The number of columns in the matrix does not match the size of the vector.");
    }

    // 稀疏矩阵-向量乘法
    struct Cooef *tmp;
    for (int idx = 0; idx < nnz; ++idx)
    {
        tmp = &cooefs[idx];
        y[tmp->i] += tmp->val * x[tmp->j];
    }
}