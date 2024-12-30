#include <SKRMatrix.h>
#include <TArray.h>
#include <Matrix.h>
#include <CSRMatrix.h>
#include <iostream>
#include <iomanip>

SKRMatrix::SKRMatrix(CSRMatrix &A)
    : Matrix(A.rows, A.rows), column_offset(A.rows + 1, 0)
{
    // 输入A假定一定是对称正定的，寻找A每一行最左端的非零元素下标
    for (int i = 0; i < rows; ++i)
    {
        int left_elm_idx = A.elm_idx[A.row_offset[i]];
        int len = i - left_elm_idx + 1;
        column_offset[i + 1] = column_offset[i] + len;
    }
    elements.resize(column_offset[A.rows]);
    elements.setAll(0.0);
};

void SKRMatrix::MVP(const Vec &x, Vec &y) const
{
    if (cols != x.size || cols != y.size)
    {
        throw std::invalid_argument("Size mismatch: The number of columns in the matrix does not match the size of the vector.");
    }
    y.setAll(0.0);

    int start;
    int len;
#pragma omp parallel for private(start, len)
    for (int row = 0; row < rows; ++row)
    {
        start = column_offset[row];
        len = column_offset[row + 1] - start;
        for (int i = 0; i < len; ++i)
        {
#pragma omp atomic
            y[row] += x[row] * elements[start + i];
        }
    }
}

void SKRMatrix::convertFromCSR(const CSRMatrix &A)
{
    // 把CSR矩阵的下三角部分转换成SKR格式方便计算Cholesky
    // 要求是从A初始化的SKR矩阵
#pragma omp parallel for
    for (int row = 0; row < rows; ++row)
    {
        int A_start = A.row_offset[row];
        int A_end = A.row_offset[row + 1];
        int SKR_start = column_offset[row];
        int SKR_len = column_offset[row + 1] - SKR_start;
        for (int i = A_start; (i < A_end) && (A.elm_idx[i] <= row); ++i)
        {
            elements[SKR_start + SKR_len - row - 1 + A.elm_idx[i]] = A.elements[i];
        }
    }
}

void SKRMatrix::print() const
{
    // 保存 std::cout 的当前格式
    std::ios old_state(nullptr);
    old_state.copyfmt(std::cout);

    std::cout << "Lower Triangular Matrix (" << rows << " x " << rows << "):" << std::endl;

    for (size_t i = 0; i < rows; ++i)
    {
        size_t start = column_offset[i];                                    // 当前行的起始位置
        size_t end = (i + 1 < rows) ? column_offset[i + 1] : elements.size; // 当前行的结束位置

        // 当前行的值
        std::vector<double> row_values(elements.begin() + start, elements.begin() + end);

        size_t idx = 0;                 // 遍历 row_values 的索引
        for (size_t j = 0; j <= i; ++j) // 只打印下三角部分，j <= i
        {
            if (idx < row_values.size() && j >= i - (row_values.size() - 1))
            {
                // 输出非零元素，保留3位小数，宽度为6
                std::cout << std::fixed << std::setprecision(2) << std::setw(6) << row_values[idx];
                ++idx;
            }
            else
            {
                // 输出零元素，保留3位小数，宽度为6
                std::cout << std::fixed << std::setprecision(2) << std::setw(6) << 0.0;
            }
        }
        std::cout << std::endl; // 换行，完成当前行的输出
    }

    // 恢复 std::cout 的原始格式
    std::cout.copyfmt(old_state);
}
