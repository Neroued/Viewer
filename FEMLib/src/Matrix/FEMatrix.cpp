#include <FEMatrix.h>
#include <TArray.h>
#include <Mesh.h>
#include <cstdint>
#include <iomanip>

NAMESPACE_BEGIN(FEMLib)

void MVP_P1_Mass(const FEMatrix &M, const Vec &x, Vec &y)
{
    y.setAll(0);

    // 先计算对角线
    for (int i = 0; i < M.rows; ++i)
    {
        y[i] = M.diag[i] * x[i];
    }

    // 根据三角形依次计算非对角线
    for (size_t t = 0; t < M.m.triangle_count(); ++t)
    {
        uint32_t a = M.m.indices[3 * t];
        uint32_t b = M.m.indices[3 * t + 1];
        uint32_t c = M.m.indices[3 * t + 2];

        // 每个三角形仅对应offdiag中的一个元素，但
        double val = M.offdiag[t];

        y[a] += val * x[b];
        y[b] += val * x[a];
        y[a] += val * x[c];
        y[c] += val * x[a];
        y[b] += val * x[c];
        y[c] += val * x[b];
        // 同时还有下三角的部分
    }
}

void MVP_P1_Sniffness(const FEMatrix &M, const Vec &x, Vec &y)
{
    y.setAll(0);

    // 先计算对角线
    for (int i = 0; i < M.rows; ++i)
    {
        y[i] = M.diag[i] * x[i];
    }

    // 根据三角形依次计算非对角线
    for (size_t t = 0; t < M.m.triangle_count(); ++t)
    {
        uint32_t a = M.m.indices[3 * t];
        uint32_t b = M.m.indices[3 * t + 1];
        uint32_t c = M.m.indices[3 * t + 2];

        // 按照AB, AC, BC的顺序存储非对角线元素
        // 因此是a行b列，a行c列，b行c列的顺序
        y[a] += M.offdiag[3 * t + 0] * x[b];
        y[b] += M.offdiag[3 * t + 0] * x[a];
        y[a] += M.offdiag[3 * t + 1] * x[c];
        y[c] += M.offdiag[3 * t + 1] * x[a];
        y[b] += M.offdiag[3 * t + 2] * x[c];
        y[c] += M.offdiag[3 * t + 2] * x[b];
        // 同时还有下三角的部分
    }
}

void FEMatrix::MVP(const Vec &x, Vec &y) const
// 按照根据不同的FEMType计算
{
    switch (femtype)
    {
    case P1_Mass:
        MVP_P1_Mass(*this, x, y);
        break;
    case P1_Stiffness:
        MVP_P1_Sniffness(*this, x, y);
    default:
        break;
    }
}

void FEMatrix::print() const
{
    // 保存 std::cout 的当前格式
    std::ios old_state(nullptr);
    old_state.copyfmt(std::cout);

    size_t n = m.vertex_count();
    std::vector<std::vector<double>> dense_matrix(n, std::vector<double>(n, 0.0));

    // 填充对角线元素
    for (size_t i = 0; i < diag.size; ++i)
    {
        dense_matrix[i][i] = diag[i];
    }

    // 填充非对角线元素
    for (size_t t = 0; t < m.triangle_count(); ++t)
    {
        uint32_t a = m.indices[3 * t + 0];
        uint32_t b = m.indices[3 * t + 1];
        uint32_t c = m.indices[3 * t + 2];

        if (femtype == P1_Mass)
        {
            double val = offdiag[t];
            dense_matrix[a][b] += val;
            dense_matrix[a][c] += val;
            dense_matrix[b][a] += val;
            dense_matrix[b][c] += val;
            dense_matrix[c][a] += val;
            dense_matrix[c][b] += val;
        }
        else if (femtype == P1_Stiffness)
        {
            dense_matrix[a][b] += offdiag[3 * t + 0];
            dense_matrix[a][c] += offdiag[3 * t + 1];
            dense_matrix[b][c] += offdiag[3 * t + 2];

            // 对称矩阵，填充反向项
            dense_matrix[b][a] += offdiag[3 * t + 0];
            dense_matrix[c][a] += offdiag[3 * t + 1];
            dense_matrix[c][b] += offdiag[3 * t + 2];
        }
    }

    // 输出稠密矩阵
    std::cout << "Dense Matrix (" << n << " x " << n << "):" << std::endl;
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            std::cout << std::fixed << std::setprecision(3) << std::setw(6) << dense_matrix[i][j];
        }
        std::cout << std::endl;
    }
    // 恢复 std::cout 的原始格式
    std::cout.copyfmt(old_state);
}

NAMESPACE_END