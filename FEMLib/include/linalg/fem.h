#pragma once

#include <NameSpace.h>
#include <FEMatrix.h>
#include <Mesh.h>
#include <NSMatrix.h>
#include <diagMatrix.h>
#include <CSRMatrix.h>

NAMESPACE_BEGIN(FEMLib)

// 求解- \Delta u + u = f

void buildMassMatrix(FEMatrix &M);
/* 根据网格建立质量矩阵
 * 对于每一个三角形，
 * 主对角线元素为 |ABC|/6 , 次对角线元素为 |ABC|/12
 * diag和offdiag均有n个元素
 * diag直接存储对角线元素
 * offdiag由于局部质量矩阵偏离对角线的元素仅有一种，因此每一个三角形仅增加一个元素
 */

void buildStiffnessMatrix(FEMatrix &S);
/* 根据网格建立刚度矩阵
 * 与质量矩阵区别在于
 * 局部刚度矩阵有6个不同元素，因此每个三角形需要占用三个offdiag的空间
 */

void addMassToStiffness(FEMatrix &S, FEMatrix &M);
// 将质量矩阵加到刚度矩阵，方便定义和使用统一的MVP

/*-------------------使用CSR矩阵建立质量和刚度矩阵-------------------*/
void buildMassMatrix(NSMatrix &M);
void buildMassMatrix(CSRMatrix &M, Mesh &mesh);

void buildStiffnessMatrix(NSMatrix &S);
void buildStiffnessMatrix(CSRMatrix &S, Mesh &mesh);

void addMassToStiffness(CSRMatrix &S, CSRMatrix &M);
// 将质量矩阵加到刚度矩阵，方便定义和使用统一的MVP

// 将CSR矩阵M的对角元素存储到D中
void buildDiagMatrix(const CSRMatrix &M, diagMatrix &D);

NAMESPACE_END