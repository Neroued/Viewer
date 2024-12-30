#pragma once

#include <CSRMatrix.h>
#include <SKRMatrix.h>
#include <TArray.h>

class Cholesky
{
public:
    SKRMatrix L;
    SKRMatrix A;
    TArray<int> minElmIdx;
    bool isInitialized;

    Cholesky();

    void attach(CSRMatrix &A_CSR);
    void attach(CSRMatrix &A_CSR, double epsilon);
    void compute();
    void solve(Vec &b, Vec &x);
};