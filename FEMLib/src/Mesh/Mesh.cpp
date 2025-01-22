#include <Mesh.h>
#include <TArray.h>
#include <cstdint>
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

int load_sphere(Mesh &mesh, int subdiv)
{
    load_cube(mesh, subdiv);
    mesh.m_meshtype = MeshType::SPHERE;

    for (size_t i = 0; i < mesh.vertex_count(); ++i)
    {
        normalize(mesh.vertex(i));
    }

    return 0;
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
    default:
        break;
    }
}

Mesh::~Mesh()
{
}

NAMESPACE_END