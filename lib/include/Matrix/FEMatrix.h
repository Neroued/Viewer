#pragma once

#include <TArray.h>
#include <Matrix.h>
#include <Mesh.h>

class FEMatrix : public Matrix
// 专门为P1元生成矩阵使用
{
public:
    enum FEMType
    {
        P1_Mass,
        P1_Stiffness
    };
    Vec diag; // 存储对角线元素
    Vec offdiag;
    FEMType femtype;
    /* offdiag存储非对角线元素
     * 对于每一个三角形，都可以构建出一个3x3的loacl质量或刚度矩阵
     * 在质量矩阵M_loc中，只有两个不同的值：对角线元素均为一个值，非对角线元素均为一个值，
     * 因此可以offdiag和diag的size是相同的
     * 对于刚度矩阵，这是一个对称矩阵，因此只需要存储上三角中的元素，即6个元素
     * diag存储3个，offdiag存储三个
     * 因此若有n个三角形
     * M.diag有n个元素, M.offdiag有n个元素
     * S.diag有n个元素，S.offdiag有3n个元素
     * 可以使用一个函数将M添加到S中方便计算
     * 同时offdiag中元素的实际位置需要与Mesh进行对应，因此需要引入对应的Mesh
     */
    Mesh &m;

    // FEMType表示矩阵的类型是质量还是刚度
    FEMatrix(Mesh &mesh, FEMType fem_type) : Matrix(mesh.vertex_count(), mesh.vertex_count()), diag(mesh.vertex_count(), 0.0), m(mesh), femtype(fem_type)
    {
        if (femtype == P1_Mass)
        {
            offdiag.resize(mesh.triangle_count());
        }
        else if (femtype == P1_Stiffness)
        {
            offdiag.resize(3 * mesh.triangle_count());
        }
        offdiag.setAll(0);
    }
    ~FEMatrix() = default;

    void MVP(const Vec &x, Vec &y) const;
    void print() const;
};

void MVP_P1_Mass(const FEMatrix &M, const Vec &x, Vec &y);

void MVP_P1_Sniffness(const FEMatrix &M, const Vec &x, Vec &y);
