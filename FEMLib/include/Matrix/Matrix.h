#pragma once

#include <NameSpace.h>
#include <TArray.h>

NAMESPACE_BEGIN(FEMLib)

class Matrix
{
public:
    int rows;
    int cols;

    Matrix() : rows(0), cols(0) {}
    Matrix(int r, int c) : rows(r), cols(c) {}

    // 纯虚函数，需要每个子类进行实现
    virtual void MVP(const Vec &x, Vec &y) const = 0;
    
    virtual ~Matrix() = default;
};

NAMESPACE_END