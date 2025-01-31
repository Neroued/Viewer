#pragma once

#include <NameSpace.h>
#include <CSRMatrix.h>
#include <SKRMatrix.h>
#include <TArray.h>

NAMESPACE_BEGIN(FEMLib)

class Cholesky
{
public:
    SKRMatrix L;
    SKRMatrix A;
    TArray<int> minElmIdx;
    bool attached = false;
    bool isInitialized = false;

    Cholesky();

    void attach(CSRMatrix &A_CSR);
    void attach(CSRMatrix &A_CSR, double epsilon);
    void compute();
    void solve(Vec &b, Vec &x);
};

NAMESPACE_END