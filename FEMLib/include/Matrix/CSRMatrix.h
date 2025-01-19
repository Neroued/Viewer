#pragma once

#include <NameSpace.h>
#include <Mesh.h>
#include <Matrix.h>
#include <TArray.h>

NAMESPACE_BEGIN(FEMLib)

class CSRMatrix : public Matrix
// 按行存储的稀疏矩阵, 存储每行不为零的元素
{
public:
    Vec elements;
    TArray<size_t> row_offset;
    TArray<size_t> elm_idx;

    CSRMatrix(int r) : Matrix(r, r), row_offset(r + 1, 0) {}
    CSRMatrix(Mesh &m); // 根据Mesh中每个顶点之间的连通性建立
    ~CSRMatrix() = default;

    void MVP(const Vec &x, Vec &y) const;
    void print() const;
    double operator()(size_t i, size_t j) const;
};

void blas_addMatrix(const CSRMatrix &M, double val, const CSRMatrix &S, CSRMatrix &A);
// 计算A = S + val * M

NAMESPACE_END