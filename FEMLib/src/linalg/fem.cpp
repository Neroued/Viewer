#include <NameSpace.h>
#include <fem.h>
#include <vec3.h>
#include <TArray.h>
#include <FEMatrix.h>
#include <CSRMatrix.h>
#include <NSMatrix.h>
#include <Mesh.h>
#include <vector>
#include <diagMatrix.h>

NAMESPACE_BEGIN(FEMLib)

static void getVector(const VertexCoord *A, const VertexCoord *B, VertexCoord *AB) // 计算向量AB
{
    AB[0] = B[0] - A[0];
    AB[1] = B[1] - A[1];
    AB[2] = B[2] - A[2];
}

static double norm(const VertexCoord *AB)
{
    return std::sqrt((double)AB[0] * AB[0] + (double)AB[1] * AB[1] + (double)AB[2] * AB[2]);
}

static double norm2(const VertexCoord *AB)
{
    return (double)AB[0] * AB[0] + (double)AB[1] * AB[1] + (double)AB[2] * AB[2];
}

static double dot(const VertexCoord *AB, const VertexCoord *AC)
{
    return AB[0] * AC[0] + AB[1] * AC[1] + AB[2] * AC[2];
}

static void cross(const VertexCoord *AB, const VertexCoord *AC, VertexCoord *ans)
{
    ans[0] = AB[1] * AC[2] - AB[2] * AC[1];
    ans[1] = AB[2] * AC[0] - AB[0] * AC[2];
    ans[2] = AB[0] * AC[1] - AB[1] * AC[0];
}

static void massLoc(const VertexCoord *AB, const VertexCoord *AC, double *Mloc);  // 根据输入两个向量代表的三角形计算局部质量矩阵
static void stiffLoc(const VertexCoord *AB, const VertexCoord *AC, double *Sloc); // 同上，计算刚度矩阵

void buildMassMatrix(FEMatrix &M)
/* 根据网格建立质量矩阵
 * 对于每一个三角形，
 * 主对角线元素为 |ABC|/6 , 次对角线元素为 |ABC|/12
 * diag和offdiag均有n个元素
 * diag直接存储对角线元素
 * offdiag由于局部质量矩阵偏离对角线的元素仅有一种，因此每一个三角形仅增加一个元素
 */
{
    Mesh &mesh = M.m;
    VertexCoord *AB = new VertexCoord[3];
    VertexCoord *AC = new VertexCoord[3];
    // 根据每个三角形进行计算
    for (size_t t = 0; t < mesh.triangle_count(); ++t)
    {
        TriangleIndex a = mesh.m_triangleIndices[3 * t + 0];
        TriangleIndex b = mesh.m_triangleIndices[3 * t + 1];
        TriangleIndex c = mesh.m_triangleIndices[3 * t + 2];

        const VertexCoord *A = mesh.vertex(a);
        const VertexCoord *B = mesh.vertex(b);
        const VertexCoord *C = mesh.vertex(c);

        getVector(A, B, AB);
        getVector(A, C, AC);

        // 局部质量矩阵，9个元素，其中只有2个不同的值, 第一个值为对角线元素，第二个值为非对角线元素
        double Mloc[2];
        massLoc(AB, AC, Mloc);

        M.diag[a] += Mloc[0];
        M.diag[b] += Mloc[0];
        M.diag[c] += Mloc[0];

        M.offdiag[t] = Mloc[1];
    }
    delete[] AB;
    delete[] AC;
}

static void massLoc(const VertexCoord *AB, const VertexCoord *AC, double *Mloc)
/* 根据输入的向量AB和AC计算局部质量矩阵
 * 首先计算三角形面积，公式为 |ABC| = 0.5 * |AB x AC|
 */
{
    VertexCoord *ans = new VertexCoord[3];
    cross(AB, AC, ans);
    double S_ABC = 0.5 * norm(ans);
    Mloc[0] = S_ABC / 6.0;
    Mloc[1] = S_ABC / 12.0;
}

void buildStiffnessMatrix(FEMatrix &S)
/* 根据网格建立刚度矩阵
 * 与质量矩阵区别在于
 * 局部刚度矩阵有6个不同元素，因此每个三角形需要占用三个offdiag的空间
 */
{
    Mesh &mesh = S.m;
    VertexCoord *AB = new VertexCoord[3];
    VertexCoord *AC = new VertexCoord[3];
    for (size_t t = 0; t < mesh.triangle_count(); ++t)
    {
        TriangleIndex a = mesh.m_triangleIndices[3 * t + 0];
        TriangleIndex b = mesh.m_triangleIndices[3 * t + 1];
        TriangleIndex c = mesh.m_triangleIndices[3 * t + 2];

        const VertexCoord *A = mesh.vertex(a);
        const VertexCoord *B = mesh.vertex(b);
        const VertexCoord *C = mesh.vertex(c);

        getVector(A, B, AB);
        getVector(A, C, AC);

        // 局部刚度矩阵，9个元素，其中有6个不同的值, 前三个依次为对角线元素，后三个依次为S_AB, S_AC, S_BC
        double Sloc[6];
        stiffLoc(AB, AC, Sloc);

        S.diag[a] += Sloc[0];
        S.diag[b] += Sloc[1];
        S.diag[c] += Sloc[2];

        S.offdiag[3 * t + 0] = Sloc[3];
        S.offdiag[3 * t + 1] = Sloc[4];
        S.offdiag[3 * t + 2] = Sloc[5];
    }
    delete[] AB;
    delete[] AC;
}

static void stiffLoc(const VertexCoord *AB, const VertexCoord *AC, double *Sloc)
/* 计算局部刚度矩阵
 * BC = AC - AB
 * 存储顺序是S_AB, S_AC, S_BC
 */
{
    // 先计算好，减少浮点运算量
    double ABAB = norm2(AB);
    double ACAC = norm2(AC);
    double ABAC = dot(AB, AC);
    double mult = 0.5 / sqrt(ABAB * ACAC - ABAC * ABAC); // 1 / 4|ABC|
    ABAB *= mult;
    ACAC *= mult;
    ABAC *= mult;

    Sloc[0] = ACAC + ABAB - 2 * ABAC;
    Sloc[1] = ACAC;
    Sloc[2] = ABAB;
    Sloc[3] = ABAC - ACAC; // S_AB
    Sloc[4] = ABAC - ABAB; // S_AC
    Sloc[5] = -ABAC;       // S_BC
}

void addMassToStiffness(FEMatrix &S, FEMatrix &M)
// 将质量矩阵加到刚度矩阵，方便定义和使用统一的MVP
{
    Mesh &mesh = S.m;
    for (size_t i = 0; i < mesh.vertex_count(); ++i)
    {
        S.diag[i] += M.diag[i];
    }
    for (size_t i = 0; i < mesh.triangle_count(); ++i)
    {
        S.offdiag[3 * i + 0] += M.offdiag[i];
        S.offdiag[3 * i + 1] += M.offdiag[i];
        S.offdiag[3 * i + 2] += M.offdiag[i];
    }
}

/*-------------------使用CSR矩阵建立质量和刚度矩阵-------------------*/
// 获取局部质量矩阵的值
double get_Mloc_value(int i, int j, const double Mloc[2])
{
    if (i == j)
        return Mloc[0]; // 对角线元素
    else
        return Mloc[1]; // 非对角线元素
}

// 构建顶点到局部索引的映射函数
std::unordered_map<uint32_t, int> build_vertex_to_local_index(uint32_t a, uint32_t b, uint32_t c)
{
    return {
        {a, 0},
        {b, 1},
        {c, 2}};
}

// 处理矩阵行的函数
void process_mass_matrix_row(CSRMatrix &M, uint32_t current_row, int i,
                             const std::unordered_map<uint32_t, int> &vertex_to_local_index,
                             const double Mloc[2])
{
    size_t offset = M.row_offset[current_row];
    size_t len = M.row_offset[current_row + 1] - offset;

    for (size_t idx = 0; idx < len; ++idx)
    {
        uint32_t col = M.elm_idx[offset + idx];

        // 检查列是否在当前三角形的顶点中
        auto it = vertex_to_local_index.find(col);
        if (it != vertex_to_local_index.end())
        {
            int j = it->second;
            double value = get_Mloc_value(i, j, Mloc);

#pragma omp atomic
            M.elements[offset + idx] += value;
        }
    }
}

void buildMassMatrix(CSRMatrix &M, Mesh &mesh)
{

#pragma omp parallel for
    for (size_t t = 0; t < mesh.triangle_count(); ++t)
    {
        VertexCoord *AB = new VertexCoord[3];
        VertexCoord *AC = new VertexCoord[3];
        TriangleIndex a = mesh.m_triangleIndices[3 * t + 0];
        TriangleIndex b = mesh.m_triangleIndices[3 * t + 1];
        TriangleIndex c = mesh.m_triangleIndices[3 * t + 2];

        const VertexCoord *A = mesh.vertex(a);
        const VertexCoord *B = mesh.vertex(b);
        const VertexCoord *C = mesh.vertex(c);

        getVector(A, B, AB);
        getVector(A, C, AC);

        // 局部质量矩阵，9个元素，其中只有2个不同的值, 第一个值为对角线元素，第二个值为非对角线元素
        double Mloc[2];
        massLoc(AB, AC, Mloc);

        uint32_t triangle[3] = {a, b, c};

        // 构建顶点到局部索引的映射
        auto vertex_to_local_index = build_vertex_to_local_index(a, b, c);

        // 遍历三角形的每一个顶点（对应全局矩阵的行）
        for (int i = 0; i < 3; ++i)
        {
            uint32_t current_row = triangle[i];
            process_mass_matrix_row(M, current_row, i, vertex_to_local_index, Mloc);
        }
        delete[] AB;
        delete[] AC;
    }
}

// 获取 Sloc 的索引函数
int get_Sloc_index(int i, int j)
{
    if (i == j)
        return i; // 对角线元素 Sloc[0], Sloc[1], Sloc[2]
    else if ((i == 0 && j == 1) || (i == 1 && j == 0))
        return 3; // Sloc[3]
    else if ((i == 0 && j == 2) || (i == 2 && j == 0))
        return 4; // Sloc[4]
    else if ((i == 1 && j == 2) || (i == 2 && j == 1))
        return 5; // Sloc[5]
    else
        return -1; // 不可能的情况
}

// 处理矩阵行的函数
void process_matrix_row(CSRMatrix &S, uint32_t current_row, int i, const uint32_t triangle[3],
                        const std::unordered_map<uint32_t, int> &vertex_to_local_index, const double Sloc[6])
{
    size_t offset = S.row_offset[current_row];
    size_t len = S.row_offset[current_row + 1] - offset;

    for (size_t idx = 0; idx < len; ++idx)
    {
        uint32_t col = S.elm_idx[offset + idx];

        // 检查列是否在当前三角形的顶点中
        auto it = vertex_to_local_index.find(col);
        if (it != vertex_to_local_index.end())
        {
            int j = it->second;
            int Sloc_index = get_Sloc_index(i, j);

            if (Sloc_index != -1)
            {
#pragma omp atomic
                S.elements[offset + idx] += Sloc[Sloc_index];
            }
        }
    }
}

void buildStiffnessMatrix(CSRMatrix &S, Mesh &mesh)
{
#pragma omp parallel for
    for (size_t t = 0; t < mesh.triangle_count(); ++t)
    {
        VertexCoord *AB = new VertexCoord[3];
        VertexCoord *AC = new VertexCoord[3];
        TriangleIndex a = mesh.m_triangleIndices[3 * t + 0];
        TriangleIndex b = mesh.m_triangleIndices[3 * t + 1];
        TriangleIndex c = mesh.m_triangleIndices[3 * t + 2];

        const VertexCoord *A = mesh.vertex(a);
        const VertexCoord *B = mesh.vertex(b);
        const VertexCoord *C = mesh.vertex(c);

        getVector(A, B, AB);
        getVector(A, C, AC);

        // 局部刚度矩阵，9个元素，其中有6个不同的值, 前三个依次为对角线元素，后三个依次为S_AB, S_AC, S_BC
        double Sloc[6];
        stiffLoc(AB, AC, Sloc);

        uint32_t triangle[3] = {a, b, c};

        // 构建顶点到局部索引的映射
        auto vertex_to_local_index = build_vertex_to_local_index(a, b, c);

        // 遍历三角形的每一个顶点（对应全局矩阵的行）
        for (int i = 0; i < 3; ++i)
        {
            uint32_t current_row = triangle[i];
            process_matrix_row(S, current_row, i, triangle, vertex_to_local_index, Sloc);
        }
        delete[] AB;
        delete[] AC;
    }
}

void addMassToStiffness(CSRMatrix &S, CSRMatrix &M)
// 将质量矩阵加到刚度矩阵，方便定义和使用统一的MVP
{
#pragma omp parallel for
    for (size_t t = 0; t < S.elements.size; ++t)
    {
        S.elements[t] += M.elements[t];
    }
}

void buildDiagMatrix(const CSRMatrix &M, diagMatrix &D)
{
    int offset;
    int len;
    int i;
    for (int r = 0; r < M.rows; ++r)
    {
        offset = M.row_offset[r];
        len = M.row_offset[r + 1] - offset;
        i = 0;
        while (i < len && M.elm_idx[offset + i] != r)
        {
            ++i;
        }
        if (i == len)
        {
            D.diag[r] = 0;
        }
        else
        {
            D.diag[r] = M.elements[offset + i];
        }
    }
}

void buildMassMatrix(NSMatrix &M)
{
    buildMassMatrix(M, M.mesh);
}

void buildStiffnessMatrix(NSMatrix &S)
{
    buildStiffnessMatrix(S, S.mesh);
}

NAMESPACE_END