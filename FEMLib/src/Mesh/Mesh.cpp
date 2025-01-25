#include <Mesh.h>
#include <TArray.h>
#include <cstdint>
#include <cmath>
#include <unordered_map>

NAMESPACE_BEGIN(FEMLib)

/* 生成立方体网格, 中心为原点，边长为2
 * 对于有n个子分割的网格
 * 正方体网格中有6n^2+2个顶点, 12n^2个三角形
 * 1. 生成顶点，同时使用hash表记录每个顶点的对应下标，生成一个不包含重复点的顶点集合 vertex_index_map
 * 2. 同时生成一个有重复点时下标与无重复点时下标的hash表 dupToNoDupIndex
 * 3. 根据dupToNoDupIndex创建三角形
 */
int load_cube(Mesh &mesh, const int subdiv)
{
    mesh.m_meshtype = MeshType::CUBE;

    int n = subdiv + 1;

    struct Face
    {
        int axis;      // 0:x, 1:y, 2:z
        int dir;       // 0:负方向, 1:正方向
        int firstAxis; // 当前面进行编号的坐标轴顺序，以保持三角形的方向性
        int lastAxis;
    };

    std::vector<Face> faces = {{0, 1, 1, 2},
                               {1, 1, 0, 2},
                               {0, 0, 1, 2},
                               {1, 0, 0, 2},
                               {2, 1, 1, 0},
                               {2, 0, 1, 0}};

    VertexIndex totalVertices = 6 * n * n;                // 重复顶点有6n^2个
    VertexIndex uniqueVertices = 6 * subdiv * subdiv + 2; // 不重复的顶点有6 * subdiv^2 + 2个

    // 使用一个hash函数将Vec3映射到int64, 避免使用Vec3作为主键
    std::unordered_map<int64_t, VertexIndex> vertex_index_map;
    vertex_index_map.reserve(uniqueVertices * 3);
    mesh.m_vertices.clear();
    mesh.m_vertices.resize(uniqueVertices * 3);

    // 从重复点索引到不重复点索引的map
    VertexIndex *dupToNoDupIndex = new VertexIndex[totalVertices];

    VertexIndex dupIndex = 0;   // 存在重复点的下标
    VertexIndex noDupIndex = 0; // 无重复点的下标

    double invSubdiv = 1.0 / (double)subdiv; // 提前计算好减少浮点数除法

    for (const Face &face : faces)
    {
        int axis = face.axis;
        int dir = face.dir;

        int idx1 = face.firstAxis;
        int idx2 = face.lastAxis;

        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                /* 先使用整数坐标表示一个点，方便后续使用hash
                 * 例如face = {0, 1}, 即表示垂直于x轴，位于正半轴区域的面
                 * axis = 0, dir = 1, idx1 = 1, idx2 = 2
                 * 经过处理后coords存储的是 [subdiv, i, j] 这样的点
                 * 之后经过处理可以变成需要的范围
                 */
                VertexIndex coords[3] = {0, 0, 0};
                coords[axis] = dir * subdiv;

                coords[idx1] = j; // 此处firstAxis上的先编号
                coords[idx2] = i;

                /* Hash函数的部分
                 * 将三个坐标映射到int64的范围中作为主键
                 * 需要subdiv < 2^20
                 */
                int64_t key = (int64_t)coords[0] | (int64_t)coords[1] << 20 | (int64_t)coords[2] << 40;

                auto it = vertex_index_map.find(key);
                if (it == vertex_index_map.end()) // 若这个点是首次出现
                {
                    vertex_index_map[key] = noDupIndex; // 存储这个点的位置为p
                    dupToNoDupIndex[dupIndex] = noDupIndex;

                    // 计算顶点的位置
                    double fx = coords[0] * invSubdiv * 2.0 - 1.0;
                    double fy = coords[1] * invSubdiv * 2.0 - 1.0;
                    double fz = coords[2] * invSubdiv * 2.0 - 1.0;

                    mesh.m_vertices[noDupIndex * 3 + 0] = fx;
                    mesh.m_vertices[noDupIndex * 3 + 1] = fy;
                    mesh.m_vertices[noDupIndex * 3 + 2] = fz;

                    ++noDupIndex;
                }
                else
                {
                    dupToNoDupIndex[dupIndex] = it->second; // 将重复点的下标对应到不重复点的下标
                }
                ++dupIndex;
            }
        }
    }
    mesh.m_triangleIndices.clear();
    mesh.m_triangleIndices.resize(36 * subdiv * subdiv); // 共计12n^2个三角形, 36n^2个顶点

    VertexIndex t = 0;
    VertexIndex faceVertexOffset = 0;
    for (int faceIdx = 0; faceIdx < 6; ++faceIdx)
    {
        for (int i = 0; i < subdiv; ++i)
        {
            for (int j = 0; j < subdiv; ++j)
            {
                VertexIndex idx0 = faceVertexOffset + i * n + j;           // 左下的点
                VertexIndex idx1 = faceVertexOffset + i * n + j + 1;       // 右下
                VertexIndex idx2 = faceVertexOffset + (i + 1) * n + j;     // 左上
                VertexIndex idx3 = faceVertexOffset + (i + 1) * n + j + 1; // 右上

                VertexIndex v0 = dupToNoDupIndex[idx0];
                VertexIndex v1 = dupToNoDupIndex[idx1];
                VertexIndex v2 = dupToNoDupIndex[idx2];
                VertexIndex v3 = dupToNoDupIndex[idx3];

                if (faceIdx == 1 || faceIdx == 2 || faceIdx == 4)
                {
                    mesh.m_triangleIndices[t++] = v1;
                    mesh.m_triangleIndices[t++] = v0;
                    mesh.m_triangleIndices[t++] = v3;
                    mesh.m_triangleIndices[t++] = v0;
                    mesh.m_triangleIndices[t++] = v2;
                    mesh.m_triangleIndices[t++] = v3;
                }
                else
                {
                    mesh.m_triangleIndices[t++] = v0;
                    mesh.m_triangleIndices[t++] = v1;
                    mesh.m_triangleIndices[t++] = v2;
                    mesh.m_triangleIndices[t++] = v1;
                    mesh.m_triangleIndices[t++] = v3;
                    mesh.m_triangleIndices[t++] = v2;
                }
            }
        }

        faceVertexOffset += n * n;
    }

    delete[] dupToNoDupIndex;

    return 0;
}

static void normalize(VertexCoord *vtx)
{
    // normalize a vertex
    VertexCoord mult = 1 / std::sqrt(vtx[0] * vtx[0] + vtx[1] * vtx[1] + vtx[2] * vtx[2]);
    vtx[0] *= mult;
    vtx[1] *= mult;
    vtx[2] *= mult;
}

int load_sphere(Mesh &mesh, const int subdiv)
{
    load_cube(mesh, subdiv);
    mesh.m_meshtype = MeshType::SPHERE;

    for (size_t i = 0; i < mesh.vertex_count(); ++i)
    {
        normalize(mesh.vertex(i));
    }

    return 0;
}

int load_square(Mesh &mesh, const int subdiv)
{
    mesh.m_meshtype = MeshType::SQUARE;

    /* ----- Vertex ----- */
    mesh.m_vertices.clear();
    mesh.m_vertices.resize(3 * (subdiv + 1) * (subdiv + 1));

    float mult = 1.0f / subdiv;
    for (int i = 0; i <= subdiv; ++i)
    {
        for (int j = 0; j <= subdiv; ++j)
        {
            auto vtx = mesh.vertex(i * (subdiv + 1) + j);
            vtx[0] = j * mult * 2 - 1;
            vtx[1] = i * mult * 2 - 1;
            vtx[2] = 0;
        }
    }

    /* ----- TriangleIndices ----- */
    mesh.m_triangleIndices.clear();
    mesh.m_triangleIndices.resize(6 * subdiv * subdiv);

    // 逆时针方向
    for (int i = 0; i < subdiv; ++i)
    {
        for (int j = 0; j < subdiv; ++j)
        {
            TriangleIndex t0 = (i + 0) * (subdiv + 1) + j;     // 左下
            TriangleIndex t1 = (i + 0) * (subdiv + 1) + j + 1; // 右下
            TriangleIndex t2 = (i + 1) * (subdiv + 1) + j + 1; // 右上
            TriangleIndex t3 = (i + 1) * (subdiv + 1) + j;     // 左上

            auto tri = mesh.triangle(2 * (i * subdiv + j));
            tri[0] = t0;
            tri[1] = t1;
            tri[2] = t2;
            tri[3] = t2;
            tri[4] = t3;
            tri[5] = t0;
        }
    }

    /* ----- BoundaryIndices ----- */
    mesh.m_boundaryIndices.clear();
    mesh.m_boundaryIndices.resize(8 * subdiv);

    // bottom, right, top, left
    auto bottom = mesh.boundary(0 * subdiv);
    auto right = mesh.boundary(1 * subdiv);
    auto top = mesh.boundary(2 * subdiv);
    auto left = mesh.boundary(3 * subdiv);
    for (int i = 0; i < subdiv; ++i)
    {
        *bottom++ = i;
        *bottom++ = i + 1;

        *right++ = (subdiv + 1) * (i + 1) - 1;
        *right++ = (subdiv + 1) * (i + 2) - 1;

        *top++ = (subdiv + 1) * (subdiv + 1) - 1 - i;
        *top++ = (subdiv + 1) * (subdiv + 1) - 1 - i - 1;

        *left++ = (subdiv + 1) * (subdiv - i);
        *left++ = (subdiv + 1) * (subdiv - i - 1);
    }
    return 0;
}

int load_hemisphere(Mesh &mesh, const int subdiv)
{
    /* 使用立方体网格 z>=0 的部分归一化生成半球面网格以及一个位于z=0的圆周边界
     * subdiv 表示侧边中短边上的边界数量，长边数量为短边的2倍
     * 1. 分别生成四个侧边，同时添加对应的边界
     * 2. 生成顶面z=1
     * 3. 顶点去重
     */

    mesh.m_meshtype = MeshType::HEMI_SPHERE;

    mesh.m_vertices.clear();
    mesh.m_vertices.resize(36 * subdiv * subdiv + 48 * subdiv + 15); // 包含重复顶点

    mesh.m_triangleIndices.clear();
    mesh.m_triangleIndices.resize(72 * subdiv * subdiv);

    mesh.m_boundaryIndices.clear();
    mesh.m_boundaryIndices.resize(16 * subdiv);

    /* ----- 生成四个侧边 ----- */
    // 将网格展开到同一个平面上，从下方的侧面开始，逆时针进行生成
    // 同时要确保生成三角形网格的orientation一致

    float mult = 0.5f / subdiv; // 此处与前面函数不同，因为subdiv含义不同

    // 第一个侧面(x, -1, z)
    /* 例如subdiv = 1，划分为形如
        3----4----5
        |    |    |
        0----1----2
        三角形顺序为(0, 1, 3), (1, 4, 3), (1, 2, 4), (2, 5, 4)
        边界为(0, 1), (1, 2)
    */

    VertexIndex vertexFaceOffset = 0;
    TriangleIndex triangleFaceOffset = 0;
    BoundaryIndex boundaryFaceOffset = 0;

    // 生成顶点
    for (int i = 0; i <= subdiv; ++i) // subdiv + 1行
    {
        for (int j = 0; j <= 2 * subdiv; ++j) // 2 * subdiv + 1列
        {
            auto vtx = mesh.vertex(vertexFaceOffset + (2 * subdiv + 1) * i + j);
            vtx[0] = j * mult * 2 - 1.0f; // x \in [-1, 1]
            vtx[1] = -1.0f;               // y = -1
            vtx[2] = i * mult * 2;        // z \in [0, 1]
        }
    }

    // 生成三角形
    for (int i = 0; i < subdiv; ++i)
    {
        for (int j = 0; j < 2 * subdiv; ++j)
        {
            TriangleIndex t0 = vertexFaceOffset + (2 * subdiv + 1) * i + j;           // 左下
            TriangleIndex t1 = vertexFaceOffset + (2 * subdiv + 1) * i + j + 1;       // 右下
            TriangleIndex t2 = vertexFaceOffset + (2 * subdiv + 1) * (i + 1) + j;     // 左上
            TriangleIndex t3 = vertexFaceOffset + (2 * subdiv + 1) * (i + 1) + j + 1; // 右上

            auto tri = mesh.triangle(triangleFaceOffset + 4 * subdiv * i + 2 * j); // 每行4 * subdiv个三角形, 一次处理两个三角形
            tri[0] = t0;
            tri[1] = t1;
            tri[2] = t2;
            tri[3] = t1;
            tri[4] = t3;
            tri[5] = t2;
        }
    }

    // 生成边界
    auto bd = mesh.boundary(boundaryFaceOffset);
    for (int i = 0; i < 2 * subdiv; ++i)
    {
        *bd++ = vertexFaceOffset + i;
        *bd++ = vertexFaceOffset + i + 1;
    }

    vertexFaceOffset += (subdiv + 1) * (2 * subdiv + 1);
    triangleFaceOffset += 4 * subdiv * subdiv;
    boundaryFaceOffset += 2 * subdiv;

    // 第二个侧面(1, y, z)
    /* 例如subdiv = 1，划分为形如
        3----4----5
        |    |    |
        0----1----2
        三角形顺序为(0, 1, 4), (0, 4, 3), (1, 2, 5), (1, 5, 4)
        边界为(0, 1), (1, 2)
    */

    // 生成顶点
    for (int i = 0; i <= subdiv; ++i)
    {
        for (int j = 0; j <= 2 * subdiv; ++j)
        {
            auto vtx = mesh.vertex(vertexFaceOffset + (2 * subdiv + 1) * i + j);
            vtx[0] = 1.0f;                // x = 1
            vtx[1] = j * mult * 2 - 1.0f; // y \in [-1, 1]
            vtx[2] = i * mult * 2;        // z \in [0, 1]
        }
    }

    // 生成三角形
    for (int i = 0; i < subdiv; ++i)
    {
        for (int j = 0; j < 2 * subdiv; ++j)
        {
            TriangleIndex t0 = vertexFaceOffset + (2 * subdiv + 1) * i + j;           // 左下
            TriangleIndex t1 = vertexFaceOffset + (2 * subdiv + 1) * i + j + 1;       // 右下
            TriangleIndex t2 = vertexFaceOffset + (2 * subdiv + 1) * (i + 1) + j;     // 左上
            TriangleIndex t3 = vertexFaceOffset + (2 * subdiv + 1) * (i + 1) + j + 1; // 右上

            auto tri = mesh.triangle(triangleFaceOffset + 4 * subdiv * i + 2 * j);
            tri[0] = t0;
            tri[1] = t1;
            tri[2] = t3;
            tri[3] = t0;
            tri[4] = t3;
            tri[5] = t2;
        }
    }

    // 生成边界
    bd = mesh.boundary(boundaryFaceOffset);
    for (int i = 0; i < 2 * subdiv; ++i)
    {
        *bd++ = vertexFaceOffset + i;
        *bd++ = vertexFaceOffset + i + 1;
    }

    vertexFaceOffset += (subdiv + 1) * (2 * subdiv + 1);
    triangleFaceOffset += 4 * subdiv * subdiv;
    boundaryFaceOffset += 2 * subdiv;

    // 第三个侧面(x, 1, z)
    /* 例如subdiv = 1，划分为形如
        3----4----5
        |    |    |
        0----1----2
        三角形顺序为(0, 3, 4), (0, 4, 1), (1, 4, 5), (1, 5, 2)
        边界为(2, 1), (1, 0)
    */

    // 生成顶点
    for (int i = 0; i <= subdiv; ++i)
    {
        for (int j = 0; j <= 2 * subdiv; ++j)
        {
            auto vtx = mesh.vertex(vertexFaceOffset + (2 * subdiv + 1) * i + j);
            vtx[0] = j * mult * 2 - 1.0f; // x \in [-1, 1]
            vtx[1] = 1.0f;                // y = 1
            vtx[2] = i * mult * 2;        // z \in [0, 1]
        }
    }

    // 生成三角形
    for (int i = 0; i < subdiv; ++i)
    {
        for (int j = 0; j < 2 * subdiv; ++j)
        {
            TriangleIndex t0 = vertexFaceOffset + (2 * subdiv + 1) * i + j;           // 左下
            TriangleIndex t1 = vertexFaceOffset + (2 * subdiv + 1) * i + j + 1;       // 右下
            TriangleIndex t2 = vertexFaceOffset + (2 * subdiv + 1) * (i + 1) + j;     // 左上
            TriangleIndex t3 = vertexFaceOffset + (2 * subdiv + 1) * (i + 1) + j + 1; // 右上

            auto tri = mesh.triangle(triangleFaceOffset + 4 * subdiv * i + 2 * j);
            tri[0] = t0;
            tri[1] = t2;
            tri[2] = t3;
            tri[3] = t0;
            tri[4] = t3;
            tri[5] = t1;
        }
    }

    // 生成边界，此处最好是倒序
    bd = mesh.boundary(boundaryFaceOffset);
    for (int i = 0; i < 2 * subdiv; ++i)
    {
        *bd++ = vertexFaceOffset + 2 * subdiv - i;
        *bd++ = vertexFaceOffset + 2 * subdiv - i - 1;
    }

    vertexFaceOffset += (subdiv + 1) * (2 * subdiv + 1);
    triangleFaceOffset += 4 * subdiv * subdiv;
    boundaryFaceOffset += 2 * subdiv;

    // 第四个侧面(-1, y, z)
    /* 例如subdiv = 1，划分为形如
        3----4----5
        |    |    |
        0----1----2
        三角形顺序为(0, 3, 1), (1, 3, 4), (1, 4, 2), (2, 4, 5)
        边界为(2, 1), (1, 0)
    */

    // 生成顶点
    for (int i = 0; i <= subdiv; ++i)
    {
        for (int j = 0; j <= 2 * subdiv; ++j)
        {
            auto vtx = mesh.vertex(vertexFaceOffset + (2 * subdiv + 1) * i + j);
            vtx[0] = -1.0f;               // x = -1
            vtx[1] = j * mult * 2 - 1.0f; // y \in [-1, 1]
            vtx[2] = i * mult * 2;        // z \in [0, 1]
        }
    }

    // 生成三角形
    for (int i = 0; i < subdiv; ++i)
    {
        for (int j = 0; j < 2 * subdiv; ++j)
        {
            TriangleIndex t0 = vertexFaceOffset + (2 * subdiv + 1) * i + j;           // 左下
            TriangleIndex t1 = vertexFaceOffset + (2 * subdiv + 1) * i + j + 1;       // 右下
            TriangleIndex t2 = vertexFaceOffset + (2 * subdiv + 1) * (i + 1) + j;     // 左上
            TriangleIndex t3 = vertexFaceOffset + (2 * subdiv + 1) * (i + 1) + j + 1; // 右上

            auto tri = mesh.triangle(triangleFaceOffset + 4 * subdiv * i + 2 * j);
            tri[0] = t0;
            tri[1] = t2;
            tri[2] = t1;
            tri[3] = t1;
            tri[4] = t2;
            tri[5] = t3;
        }
    }

    // 生成边界，此处最好是倒序
    bd = mesh.boundary(boundaryFaceOffset);
    for (int i = 0; i < 2 * subdiv; ++i)
    {
        *bd++ = vertexFaceOffset + 2 * subdiv - i;
        *bd++ = vertexFaceOffset + 2 * subdiv - i - 1;
    }

    vertexFaceOffset += (subdiv + 1) * (2 * subdiv + 1);
    triangleFaceOffset += 4 * subdiv * subdiv;
    boundaryFaceOffset += 2 * subdiv;

    // 顶面(x, y, 1)
    /* 例如subdiv = 1，划分为形如
        6----7----8
        |    |    |
        3----4----5
        |    |    |
        0----1----2
        三角形顺序为(0, 1, 3), (1, 4, 3), (1, 2, 4), (2, 5, 4)
        无边界
    */

    // 生成顶点
    for (int i = 0; i <= 2 * subdiv; ++i)
    {
        for (int j = 0; j <= 2 * subdiv; ++j)
        {
            auto vtx = mesh.vertex(vertexFaceOffset + (2 * subdiv + 1) * i + j);
            vtx[0] = j * mult * 2 - 1.0f;
            vtx[1] = i * mult * 2 - 1.0f;
            vtx[2] = 1;
        }
    }

    // 生成三角形
    for (int i = 0; i < 2 * subdiv; ++i)
    {
        for (int j = 0; j < 2 * subdiv; ++j)
        {
            TriangleIndex t0 = vertexFaceOffset + (2 * subdiv + 1) * i + j;           // 左下
            TriangleIndex t1 = vertexFaceOffset + (2 * subdiv + 1) * i + j + 1;       // 右下
            TriangleIndex t2 = vertexFaceOffset + (2 * subdiv + 1) * (i + 1) + j;     // 左上
            TriangleIndex t3 = vertexFaceOffset + (2 * subdiv + 1) * (i + 1) + j + 1; // 右上

            auto tri = mesh.triangle(triangleFaceOffset + 4 * subdiv * i + 2 * j);
            tri[0] = t0;
            tri[1] = t1;
            tri[2] = t2;
            tri[3] = t1;
            tri[4] = t3;
            tri[5] = t2;
        }
    }

    // 顶点去重
    deduplicateVertices(mesh);

    // 归一化顶点
    for (size_t i = 0; i < mesh.vertex_count(); ++i)
    {
        normalize(mesh.vertex(i));
    }

    return 0;
}

// 将 (ix,iy,iz) 打包到 64 位整数中，
// 假设 ix,iy,iz ∈ [-2^20, 2^20)
static inline int64_t packInt3ToInt64(int ix, int iy, int iz)
{
    // 平移到 [0, 2^21) 区间
    const int64_t base = (1LL << 20); // 2^20

    int64_t X = ix + base; // \in [0, 2^21)
    int64_t Y = iy + base;
    int64_t Z = iz + base;

    // 做掩码以防越界
    X &= 0x1FFFFF; // 21 bits
    Y &= 0x1FFFFF;
    Z &= 0x1FFFFF;

    // 按位合成 [Z(21bits) | Y(21bits) | X(21bits)]
    int64_t key = (X) | (Y << 21) | (Z << 42);

    return key;
}

// 将三个浮点数 (x,y,z) 按照给定 scale 转为整数
// 然后调用 packInt3ToInt64() 打包成 64 位整数
static inline int64_t packVertexCoord3ToInt64(VertexCoord x, VertexCoord y, VertexCoord z, float scale)
{
    // 先按 scale 缩放，并取 llround() 保证是四舍五入
    int ix = (int)std::llround(x * scale);
    int iy = (int)std::llround(y * scale);
    int iz = (int)std::llround(z * scale);

    // 然后打包
    return packInt3ToInt64(ix, iy, iz);
}

void deduplicateVertices(Mesh &mesh)
{
    float scale = 1e5f;
    std::unordered_map<int64_t, VertexIndex> map;
    decltype(mesh.m_vertices) noDupVertices;

    VertexIndex newIndexCount = 0;
    std::vector<VertexIndex> oldToNew(mesh.vertex_count());
    std::vector<VertexIndex> newToOld;
    for (VertexIndex i = 0; i < mesh.vertex_count(); ++i)
    {
        auto vtx = mesh.vertex(i);
        int64_t key = packVertexCoord3ToInt64(vtx[0], vtx[1], vtx[2], scale);
        auto it = map.find(key);
        if (it == map.end()) // 首次出现
        {
            map[key] = newIndexCount;
            oldToNew[i] = newIndexCount;
            newToOld.push_back(i);
            ++newIndexCount;
        }
        else
        {
            oldToNew[i] = map[key];
        }
    }

    std::size_t numNoDupVtx = map.size();
    noDupVertices.resize(numNoDupVtx * 3);
    for (VertexIndex i = 0; i < numNoDupVtx; ++i)
    {
        auto v = mesh.vertex(newToOld[i]);
        noDupVertices[3 * i + 0] = v[0];
        noDupVertices[3 * i + 1] = v[1];
        noDupVertices[3 * i + 2] = v[2];
    }
    mesh.m_vertices = noDupVertices;

    for (TriangleIndex i = 0; i < mesh.triangle_count(); ++i)
    {
        auto tri = mesh.triangle(i);
        tri[0] = oldToNew[tri[0]];
        tri[1] = oldToNew[tri[1]];
        tri[2] = oldToNew[tri[2]];
    }

    for (BoundaryIndex i = 0; i < mesh.boundary_count(); ++i)
    {
        auto bd = mesh.boundary(i);
        bd[0] = oldToNew[bd[0]];
        bd[1] = oldToNew[bd[1]];
    }
}

Mesh::Mesh(int subdiv, MeshType meshtype)
{
    switch (meshtype)
    {
    case MeshType::CUBE:
        load_cube(*this, subdiv);
        break;
    case MeshType::SPHERE:
        load_sphere(*this, subdiv);
        break;
    case MeshType::SQUARE:
        load_square(*this, subdiv);
        break;
    case MeshType::HEMI_SPHERE:
        load_hemisphere(*this, subdiv);
        break;
    default:
        break;
    }
}

Mesh::~Mesh()
{
}

NAMESPACE_END
