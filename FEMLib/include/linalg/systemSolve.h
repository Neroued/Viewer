#pragma once

#include <NameSpace.h>
#include <TArray.h>
#include <Matrix.h>
#include <COOMatrix.h>
#include <iostream>

NAMESPACE_BEGIN(FEMLib)

bool decentGradientSolve(Matrix &A, Vec &B, Vec &u, Vec &r, Vec &Ar, double *rel_error, int *iter, double tol, int iterMax = 1000);
/* Input:
 * Matrix &A : 线性方程组的矩阵
 * Vec &B: 右端项
 * Vec &u: 解，大小需要合适
 * Vec &r: 存放residue的向量
 * Vec &Ar: 存放Ar的向量
 * double *rel_error: 返回最终结果的误差
 * int *iter: 返回迭代的次数
 * double tol: 容许误差
 * int iterMax: 最大迭代次数
 */

bool conjugateGradientSolve(Matrix &A, Vec &B, Vec &u, Vec &r, Vec &p, Vec &Ap, double *rel_error, int *iter, double tol, int iterMax = 1000);
/* Input:
 * Matrix &A : 线性方程组的矩阵
 * Vec &B: 右端项
 * Vec &u: 解，大小需要合适
 * Vec &r: 存放residue的向量
 * Vec &p: 存放p的向量
 * Vec &Ap: 存放Ap的向量
 * double *rel_error: 返回最终结果的误差
 * int *iter: 返回迭代的次数
 * double tol: 容许误差
 * int iterMax: 最大迭代次数
 */

bool decentGradientSolve(COOMatrix &M, COOMatrix &S, Vec &B, Vec &u, double tol, int iterMax = 1000);

bool conjugateGradientSolve(COOMatrix &M, COOMatrix &S, Vec &B, Vec &u, double tol, int iterMax = 1000);

NAMESPACE_END