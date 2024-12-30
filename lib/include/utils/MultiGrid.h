#pragma once

#include <NSMatrix.h>
#include <Mesh.h>
#include <TArray.h>
#include <diagMatrix.h>

/* 多重网格法对有限元线性系统进行求解 Ax = b
 * 根据输入的初始网格信息生成三重粗网格
 * 使用输入的矩阵生成方法，在每个粗网格上生成矩阵
 * 对于粗网格，每一层的subdiv是前一层的一半
 * 要求初始网格subdiv为8的倍数
 * 此时对于b，粗网格的点都是原本细分网格的顶点，因此直接使用对应位置的值
 * 坑：类成员使用初始化列表初始化时是根据成员在类中定义的顺序来的，而不是根据初始化列表的顺序
 * 因此假如初始化列表中后一项依赖之前项的初始化，应该在类成员声明时就考虑好顺序
 */
class MultiGrid
{
public:
    MeshType mt;
    int subdiv;
    double w;
    double tol;

    Mesh &m0; // 原本的网格
    Mesh m1;  // 双重细分网格
    Mesh m2;
    Mesh m3;

    NSMatrix A0; // 分别对应每个网格上的矩阵
    NSMatrix A1;
    NSMatrix A2;
    NSMatrix A3;

    diagMatrix D0;

    Vec r0;
    Vec r1;
    Vec r2;
    Vec r3;

    MultiGrid(Mesh &mesh, void funcBuildMatrix(NSMatrix &M));
    void solve(Vec &b, Vec &u);
    void setOmega(double val) { w = val; }

    // 需要来自各个网格的顶点对应信息来将b映射到各个粗网格上
    // 因此使用在网格构建过程中得到的dupToNoDupIndex

    void projToCoarse(Vec &b, Mesh &m0, Vec &b1, Mesh &m1);                                                        // 将b从细网格m0映射到粗网格m1上，结果在b1中
    void projToFine(Vec &b, Mesh &m0, Vec &b1, Mesh &m1);                                                          // 将b从粗网格m0映射到细网格m1上
    void dumpedJacobi(const NSMatrix &A, const diagMatrix &D, const Vec &b, Vec &x, Vec &r, int iter);            // 重稀疏Jacobi平滑器
    void conjugateGraidentSmooth(NSMatrix &A, Vec &b, Vec &x, int iter); // 共轭梯度平滑
    void setZeroMean(Vec &x);
};