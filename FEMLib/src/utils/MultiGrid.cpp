// #include <MultiGrid.h>
// #include <stdexcept>
// #include <iostream>
// #include <cmath>
// #include <fem.h>
// #include <systemSolve.h>
// #include <timer.h>

// NAMESPACE_BEGIN(FEMLib)

// MultiGrid::MultiGrid(Mesh &mesh, void funcBuildMatrix(NSMatrix &M))
//     : mt(mesh.meshtype), subdiv(mesh.subdiv), w(0.6),
//       m0(mesh), m1(subdiv / 2, mt, true), m2(subdiv / 4, mt, true), m3(subdiv / 8, mt, true),
//       A0(m0), A1(m1), A2(m2), A3(m3),
//       D0(A0.rows),
//       r0(A0.rows, 0.0), r1(m1.vertex_count(), 0.0), r2(m2.vertex_count(), 0.0), r3(A3.rows, 0.0)
// {
//     tol = 1e-6;
//     // 根据传入的函数构建矩阵
//     funcBuildMatrix(A0);
//     funcBuildMatrix(A1);
//     funcBuildMatrix(A2);
//     funcBuildMatrix(A3);

//     // 构建对角矩阵
//     buildDiagMatrix(A0, D0);
// }

// void MultiGrid::projToCoarse(Vec &b, Mesh &m0, Vec &b1, Mesh &m1)
// /* 将b从细网格投影到粗网格上
//  * 根据subdiv选取的特性，粗网格的顶点也是细网格的顶点
//  * 直接使用对应顶点的值
//  */
// {
//     int subdivFine = m0.subdiv;
//     int subdivCoarse = m1.subdiv;
//     int *dd0 = m0.dupToNoDupIndex;
//     int *dd1 = m1.dupToNoDupIndex;

//     int N_fine = subdivFine + 1;
//     int N_coarse = subdivCoarse + 1;
//     int step = subdivFine / subdivCoarse;

//     // 粗网格中的点 (row_c, col_c) 对应细网格中的点(row_c *step, col_c * step)
//     for (int face = 0; face < 6; ++face)
//     {
//         int faceOffsetCoarse = face * N_coarse * N_coarse;
//         int faceOffsetFine = face * N_fine * N_fine;
//         // 遍历粗网格的行
//         for (int row_c = 0; row_c < N_coarse; ++row_c)
//         {
//             int row_f = row_c * step;
//             for (int col_c = 0; col_c < N_coarse; ++col_c)
//             {
//                 int col_f = col_c * step;

//                 int i = faceOffsetCoarse + row_c * N_coarse + col_c;
//                 int t = faceOffsetFine + row_f * N_fine + col_f;

//                 b1[dd1[i]] = b[dd0[t]];
//             }
//         }
//     }
// }

// void MultiGrid::projToFine(Vec &b, Mesh &m0, Vec &b1, Mesh &m1)
// // 将b从粗网格投影到细网格
// {
//     int subdivFine = m1.subdiv;
//     int subdivCoarse = m0.subdiv;
//     int *ddFine = m1.dupToNoDupIndex;
//     int *ddCoarse = m0.dupToNoDupIndex;

//     int N_Fine = subdivFine + 1;
//     int N_Coarse = subdivCoarse + 1;
//     int step = subdivFine / subdivCoarse;

//     for (int face = 0; face < 6; ++face)
//     {
//         int faceOffsetCoarse = face * N_Coarse * N_Coarse;
//         int faceOffsetFine = face * N_Fine * N_Fine;

//         for (int row_f = 0; row_f < N_Fine; ++row_f)
//         {
//             // 计算当前行对应的粗网格行，以及上下两行
//             float row_c_f = (float)row_f / step;
//             int row_c0 = (int)std::floor(row_c_f);           // 细网格行对应的粗网格的下方一行
//             int row_c1 = std::min(row_c0 + 1, N_Coarse - 1); // 上方一行，需要避免超出范围
//             float dy = row_c_f - row_c0;                     // 用于双线性插值

//             for (int col_f = 0; col_f < N_Fine; ++col_f)
//             {
//                 // 类似的计算当前列对应的粗网格的列
//                 float col_c_f = (float)col_f / step;
//                 int col_c0 = (int)std::floor(col_c_f);
//                 int col_c1 = std::min(col_c0 + 1, N_Coarse - 1);
//                 float dx = col_c_f - col_c0;

//                 int t = faceOffsetFine + row_f * N_Fine + col_f;
//                 int idx_f = ddFine[t];

//                 if (row_f % step == 0 && col_f % step == 0) // 细网格点与粗网格点重合，直接使用粗网格的值
//                 {
//                     int i = faceOffsetCoarse + row_c0 * N_Coarse + col_c0;
//                     b1[idx_f] = b[ddCoarse[i]];
//                 }
//                 else
//                 {
//                     // 使用双线性插值
//                     int i00 = faceOffsetCoarse + row_c0 * N_Coarse + col_c0;
//                     int i01 = faceOffsetCoarse + row_c1 * N_Coarse + col_c0;
//                     int i10 = faceOffsetCoarse + row_c0 * N_Coarse + col_c1;
//                     int i11 = faceOffsetCoarse + row_c1 * N_Coarse + col_c1;

//                     int idx00 = ddCoarse[i00];
//                     int idx01 = ddCoarse[i01];
//                     int idx10 = ddCoarse[i10];
//                     int idx11 = ddCoarse[i11];

//                     double v00 = b[idx00];
//                     double v01 = b[idx01];
//                     double v10 = b[idx10];
//                     double v11 = b[idx11];

//                     // 双线性插值，先对x进行插值，再对y进行插值
//                     double v0 = v00 * (1 - dx) + v10 * dx;
//                     double v1 = v01 * (1 - dx) + v11 * dx;
//                     double v = v0 * (1 - dy) + v1 * dy;

//                     b1[idx_f] = v;
//                 }
//             }
//         }
//     }
// }

// void MultiGrid::dumpedJacobi(const NSMatrix &A, const diagMatrix &D, const Vec &b, Vec &x, Vec &r, int iter = 5)
// {
//     Vec p(x.size); // 临时空间

//     for (int i = 0; i < iter; ++i)
//     {
//         A.MVP(x, p);
//         blas_axpby(1.0, b, -1.0, p, r); // r = b - Ax
//         D.MVP_inverse(r, p);
//         blas_axpby(1.0, x, w, p, x); // x = x + w * D^-1 * r
//     }
// }

// void MultiGrid::conjugateGraidentSmooth(NSMatrix &A, Vec &b, Vec &x, int iter = 5)
// {
//     int cg_iter;
//     double cg_rel_error;
//     Vec r(b.size), p(b.size), Ap(b.size);
//     conjugateGradientSolve(A, b, x, r, p, Ap, &cg_rel_error, &cg_iter, tol, iter);
// }

// /* 我们希望在最细网格上求解Ax = b
//  * 预平滑后计算残差，将残差限制到下一层网格，求解Ae = r得到误差e
//  * 将e插值回到细网格，更新x = x + e
//  */
// void MultiGrid::solve(Vec &b, Vec &x)
// {
//     double b_norm = b.norm();
//     double rel_error;
//     int iter = 0;
//     int iterMax = 1000;
//     Vec p0(x.size);
//     Vec p3(r3.size), Ap3(r3.size), t3(r3.size);
//     Vec e3(r3.size, 0.0), e2(r2.size, 0.0), e1(r1.size, 0.0);
//     while (iter++ < iterMax)
//     {
//         // 计算残差并限制
//         // conjugateGraidentSmooth(A0, b, x, 10); // 预平滑
//         dumpedJacobi(A0, D0, b, x, r0);

//         A0.MVP(x, p0);
//         blas_axpby(1.0, b, -1.0, p0, r0); // 计算残差
//         rel_error = r0.norm() / b_norm;
//         std::cout << "iter :" << iter << " rel_error: " << rel_error << std::endl;

//         if (rel_error < this->tol)
//             break;

//         projToCoarse(r0, m0, r1, m1); // 限制到粗网格m1
//         projToCoarse(r1, m1, r2, m2);
//         projToCoarse(r2, m2, r3, m3);
//         setZeroMean(r3);

//         // 在最粗网格上直接求解Ae = r
//         int cg_iter;
//         double cg_rel_error;
//         conjugateGradientSolve(A3, r3, e3, t3, p3, Ap3, &cg_rel_error, &cg_iter, tol);
//         // std::cout << "cg_rel_error: " << cg_rel_error << std::endl;

//         // 插值回到细网格
//         projToFine(e3, m3, e2, m2);
//         projToFine(e2, m2, e1, m1);
//         projToFine(e1, m1, p0, m0);
//         setZeroMean(p0);
//         blas_axpby(1.0, x, 1.0, p0, x);

//         // conjugateGraidentSmooth(A0, b, x, 10); // 后平滑
//         dumpedJacobi(A0, D0, b, x, r0);
//     }
// }

// void MultiGrid::setZeroMean(Vec &x)
// {
//     double mean = x.sum() / (double)x.size;

//     for (size_t t = 0; t < x.size; ++t)
//     {
//         x[t] -= mean;
//     }
// }

// // void MultiGrid::solve(Vec &b, Vec &x)
// // {
// //     Vec x1(A1.rows, 0.0), x2(A2.rows, 0.0), x3(A3.rows, 0.0);
// //     Vec p1(A1.rows, 0.0), p2(A2.rows, 0.0), p3(A3.rows, 0.0);
// //     Vec Ap1(A1.rows, 0.0), Ap2(A2.rows, 0.0), Ap3(A3.rows, 0.0);
// //     Vec b1(A1.rows, 0.0), b2(A2.rows, 0.0), b3(A3.rows, 0.0);
// //     Vec p0(x.size), Ap0(x.size);
// //     projToCoarse(b, m0, b1, m1);
// //     projToCoarse(b, m0, b2, m2);
// //     projToCoarse(b, m0, b3, m3);
// //     setZeroMean(b1);
// //     setZeroMean(b2);
// //     setZeroMean(b3);

// //     double rel_err;
// //     int iter;
// //     conjugateGradientSolve(A3, b3, x3, r3, p3, Ap3, &rel_err, &iter, tol, 10000);
// //     std::cout << "iter: " << iter << " rel_err: " << rel_err <<std::endl;
// //     projToFine(x3, m3, x2, m2);
// //     conjugateGradientSolve(A2, b2, x2, r2, p2, Ap2, &rel_err, &iter, tol, 10000);
// //     std::cout << "iter: " << iter << " rel_err: " << rel_err <<std::endl;
// //     projToFine(x2, m2, x1, m1);
// //     conjugateGradientSolve(A1, b1, x1, r1, p1, Ap1, &rel_err, &iter, tol, 10000);
// //     std::cout << "iter: " << iter << " rel_err: " << rel_err << std::endl;
// //     projToFine(x1, m1, x, m0);
// //     conjugateGradientSolve(A0, b, x, r0, p0, Ap0, &rel_err, &iter, tol, 10000);
// //     std::cout << "iter: " << iter << " rel_err: " << rel_err << std::endl;
// // }

// NAMESPACE_END