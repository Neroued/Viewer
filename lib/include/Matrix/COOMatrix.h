#pragma once

#include <Matrix.h>
#include <TArray.h>
#include <iostream>
#include <stdexcept>
#include <vector>

typedef struct Cooef
{
    int i;
    int j;
    double val;
} Cooef;

class COOMatrix : public Matrix
// 按元素存储的稀疏矩阵
{
public:
    int nnz; //  非零元素的数量
    struct Cooef *cooefs;

    COOMatrix() : Matrix(0, 0), nnz(0), cooefs(nullptr) {}
    COOMatrix(int r, int c) : Matrix(r, c), nnz(0), cooefs(nullptr) {}
    COOMatrix(int r, int c, int nonZeroElements)
        : Matrix(r, c), nnz(nonZeroElements), cooefs(new Cooef[nonZeroElements]) {}

    ~COOMatrix() { delete[] cooefs; }

    // 禁用拷贝构造和赋值运算符
    COOMatrix(const COOMatrix &) = delete;
    COOMatrix &operator=(const COOMatrix &) = delete;

    void MVP(const Vec &x, Vec &y) const; // Ax = y
};