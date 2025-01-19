#include "Object.h"
#include "MaterialManager.h"
#include <QDebug>
#include <QFile>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "Mesh.h" // 如果需要把 Mesh 解析为顶点索引
using namespace FEMLib;

Object::Object(QObject *parent)
    : QObject(parent)
    , m_position(0.0f, 0.0f, 0.0f), m_rotation(), m_scale(1.0f, 1.0f, 1.0f)
    , m_shouldUpdateModelMatrix(true)
    , m_shaderName("basic"), m_shader(nullptr)
    , m_drawMode(DrawMode::FILL), m_objectType(ObjectType::STATIC)
    , m_materialName("default"), m_material(nullptr)
{
}

Object::~Object()
{
}

void Object::createBuffers()
{
    // Initialize Buffers
    m_vertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vertexBuffer.create();

    m_indexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_indexBuffer.create();

    m_colorBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_colorBuffer.create();

    m_normalBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_normalBuffer.create();

    m_tangentBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_tangentBuffer.create();

    m_biTangentBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_biTangentBuffer.create();

    m_texCoordBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_texCoordBuffer.create();
}

void Object::initialize()
{
    initializeOpenGLFunctions();

    // Initialize VAO
    if (!m_vao.create())
    {
        qWarning() << "[Object] Failed to create VAO!";
    }

    // 创建并上传至buffer
    createBuffers();
    uploadToBuffer();

    // 获取shader
    m_shader = ShaderManager::instance()->getShader(m_shaderName);

    // 获取material
    m_material = MaterialManager::instance()->getMaterial(m_materialName);

    m_initialized = true;
}

// 变换相关
void Object::setPosition(const QVector3D &pos)
{
    m_position = pos;
    m_shouldUpdateModelMatrix = true;
}
void Object::setRotation(const QQuaternion &rot)
{
    m_rotation = rot;
    m_shouldUpdateModelMatrix = true;
}
void Object::setScale(const QVector3D &scl)
{
    m_scale = scl;
    m_shouldUpdateModelMatrix = true;
}
void Object::updateModelMatrix()
{
    m_modelMatrix.setToIdentity();
    m_modelMatrix.translate(m_position);
    m_modelMatrix.rotate(m_rotation);
    m_modelMatrix.scale(m_scale);

    m_shouldUpdateModelMatrix = false;
}
QMatrix4x4 Object::getModelMatrix()
{
    if (m_shouldUpdateModelMatrix)
    {
        updateModelMatrix();
    }
    return m_modelMatrix;
}

// Helper function to normalize a vector
void normalize(std::vector<float> &vec)
{
    float length = std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    if (length > 0.0f)
    {
        vec[0] /= length;
        vec[1] /= length;
        vec[2] /= length;
    }
}

void Object::loadFromMesh(const Mesh &mesh)
{
    m_vertices.resize(mesh.vertices.size * 3);
    m_indices.resize(mesh.indices.size);
    m_normals.resize(mesh.vertices.size * 3, 0.0f);

    std::size_t k = 0;
    for (std::size_t i = 0; i < mesh.vertices.size; ++i)
    {
        m_vertices[k++] = (float)mesh.vertices[i].x;
        m_vertices[k++] = (float)mesh.vertices[i].y;
        m_vertices[k++] = (float)mesh.vertices[i].z;
    }

    for (std::size_t i = 0; i < mesh.indices.size; ++i)
    {
        m_indices[i] = mesh.indices[i];
    }

    // Step 3: Compute face normals and accumulate to vertex normals
    for (std::size_t i = 0; i < mesh.indices.size; i += 3)
    {
        // Get triangle vertex indices
        std::size_t idx0 = mesh.indices[i];
        std::size_t idx1 = mesh.indices[i + 1];
        std::size_t idx2 = mesh.indices[i + 2];

        // Get triangle vertices
        float v0[3] = {m_vertices[idx0 * 3], m_vertices[idx0 * 3 + 1], m_vertices[idx0 * 3 + 2]};
        float v1[3] = {m_vertices[idx1 * 3], m_vertices[idx1 * 3 + 1], m_vertices[idx1 * 3 + 2]};
        float v2[3] = {m_vertices[idx2 * 3], m_vertices[idx2 * 3 + 1], m_vertices[idx2 * 3 + 2]};

        // Compute two edges of the triangle
        float edge1[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
        float edge2[3] = {v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};

        // Compute face normal using cross product
        float faceNormal[3] = {
            edge1[1] * edge2[2] - edge1[2] * edge2[1],
            edge1[2] * edge2[0] - edge1[0] * edge2[2],
            edge1[0] * edge2[1] - edge1[1] * edge2[0]};

        // Accumulate face normal to each vertex of the triangle
        for (int j = 0; j < 3; ++j)
        {
            m_normals[idx0 * 3 + j] += faceNormal[j];
            m_normals[idx1 * 3 + j] += faceNormal[j];
            m_normals[idx2 * 3 + j] += faceNormal[j];
        }
    }

    // Step 4: Normalize all vertex normals
    for (std::size_t i = 0; i < mesh.vertices.size; ++i)
    {
        std::vector<float> normal = {
            m_normals[i * 3],
            m_normals[i * 3 + 1],
            m_normals[i * 3 + 2]};
        normalize(normal);
        m_normals[i * 3] = normal[0];
        m_normals[i * 3 + 1] = normal[1];
        m_normals[i * 3 + 2] = normal[2];
    }
}

bool Object::loadFromGLB(const QString &filePath)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string warn, err;

    // 使用 QFile 从 QRC 中读取文件内容
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open GLB file from QRC:" << filePath;
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // 将 QByteArray 转换为 std::vector<unsigned char>
    std::vector<unsigned char> data(fileData.begin(), fileData.end());

    // 使用 TinyGLTF 加载二进制数据
    bool success_load = loader.LoadBinaryFromMemory(&model, &err, &warn, data.data(), data.size());
    if (!success_load)
    {
        qWarning() << "Failed to parse GLB file:" << QString::fromStdString(err);
        return false;
    }

    // 提取网格信息（仅处理第一个网格）
    if (model.meshes.empty() || model.meshes[0].primitives.empty())
    {
        qWarning() << "No meshes found in GLB file.";
        return false;
    }

    const auto &primitive = model.meshes[0].primitives[0];

    // 解析顶点数据
    if (primitive.attributes.find("POSITION") != primitive.attributes.end())
    {
        const auto &accessor = model.accessors[primitive.attributes.at("POSITION")];
        const auto &bufferView = model.bufferViews[accessor.bufferView];
        const auto &buffer = model.buffers[bufferView.buffer];

        const float *positions = reinterpret_cast<const float *>(
            &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

        m_vertices.clear();
        for (size_t i = 0; i < accessor.count * 3; ++i)
        {
            m_vertices.push_back(positions[i]);
        }
    }
    else
    {
        qWarning() << "No POSITION attribute found in mesh.";
        return false;
    }

    // 解析索引数据
    if (primitive.indices >= 0)
    {
        const auto &accessor = model.accessors[primitive.indices];
        const auto &bufferView = model.bufferViews[accessor.bufferView];
        const auto &buffer = model.buffers[bufferView.buffer];

        m_indices.clear();
        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            const unsigned short *indices = reinterpret_cast<const unsigned short *>(
                &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
            for (size_t i = 0; i < accessor.count; ++i)
            {
                m_indices.push_back(indices[i]);
            }
        }
        else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
        {
            const unsigned int *indices = reinterpret_cast<const unsigned int *>(
                &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
            for (size_t i = 0; i < accessor.count; ++i)
            {
                m_indices.push_back(indices[i]);
            }
        }
        else
        {
            qWarning() << "Unsupported index component type.";
            return false;
        }
    }
    else
    {
        qWarning() << "No indices found in mesh.";
        return false;
    }

    return true;
}

void Object::setMaterial(const QString &materialName)
{
    if (m_initialized)
    {
        auto m = MaterialManager::instance()->getMaterial(materialName);
        if (m)
        {
            m_material = m;
            m_materialName = materialName;
        }
        else
        {
            qWarning() << "Warning: Material name: " << materialName << " Not Exist!";
        }
    }
    else
    {
        m_materialName = materialName;
    }
}
void Object::setShader(const QString &shaderName)
{

    if (m_initialized) // 表示Object已创建
    {
        auto s = ShaderManager::instance()->getShader(shaderName);
        if (s)
        {
            m_shader = s;
            m_shaderName = shaderName;
        }
        else
        {
            qWarning() << "Warning: Shader name: " << shaderName << " Not Exist!";
        }
    }
    else // 首次创建，在初始化中绑定shader
    {
        m_shaderName = shaderName;
    }
}

void Object::setDrawMode(DrawMode mode)
{
    m_drawMode = mode;
}
DrawMode Object::getDrawMode() const
{
    return m_drawMode;
}

void Object::setObjectType(ObjectType type)
{
    m_objectType = type;
}
ObjectType Object::getObjectType() const
{
    return m_objectType;
}

void Object::setColorBuffer(const std::vector<float> &colorData)
{
    m_colorBufferData = colorData;
}

static void fillData(std::vector<float> &data, std::size_t targetSize, std::vector<float> defaultValue)
// 将data补充至目标大小
// 若为空则使用默认值填充
{
    if (!data.empty())
    {
        std::size_t count = data.size() / 3;

        if (count < targetSize)
        {
            data.reserve(targetSize * 3);
            float last[3] = {data[count * 3 - 3], data[count * 3 - 2], data[count * 3 - 1]};

            for (std::size_t i = count; i < targetSize; ++i)
            {
                data.push_back(last[0]);
                data.push_back(last[1]);
                data.push_back(last[2]);
            }
        }
    }
    else
    {
        data.resize(targetSize * 3);
        for (std::size_t i = 0; i < targetSize; ++i)
        {
            data[i + 0] = defaultValue[0];
            data[i + 1] = defaultValue[1];
            data[i + 2] = defaultValue[2];
        }
    }
}

// ---------------------------------------------------------
// Buffer upload / update
// ---------------------------------------------------------
void Object::uploadToBuffer()
{
    // 绑定VAO
    m_vao.bind();

    /* ===== Vertex Buffer ===== */
    /* ====== location = 0 ======*/
    m_vertexBuffer.bind();
    QOpenGLBuffer::UsagePattern usage = QOpenGLBuffer::StaticDraw;
    m_vertexBuffer.setUsagePattern(usage);
    m_vertexBuffer.allocate(m_vertices.data(), int(m_vertices.size() * sizeof(float)));
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    std::size_t vertexCount = m_vertices.size() / 3;

    /* ===== Index Buffer ===== */
    m_indexBuffer.bind();
    m_indexBuffer.setUsagePattern(usage);
    m_indexBuffer.allocate(m_indices.data(), int(m_indices.size() * sizeof(unsigned int)));
    // Note: QOpenGLBuffer::IndexBuffer is automatically bound to GL_ELEMENT_ARRAY_BUFFER

    /* ===== Normal Buffer ===== */
    /* ====== location = 1 ======*/
    m_normalBuffer.bind();
    m_normalBuffer.setUsagePattern(usage);
    fillData(m_normals, vertexCount, {0.0f, 0.0f, 1.0f});
    m_normalBuffer.allocate(m_normals.data(), int(m_normals.size() * sizeof(float)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    /* ===== Tangent Buffer ===== */
    /* ======= location = 2 ======*/
    m_tangentBuffer.bind();
    m_tangentBuffer.setUsagePattern(usage);
    fillData(m_tangent, vertexCount, {1.0f, 0.0f, 0.0f});
    m_tangentBuffer.allocate(m_tangent.data(), int(m_tangent.size() * sizeof(float)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);

    /* ===== biTangent Buffer ===== */
    /* ======= location = 3 ======*/
    m_biTangentBuffer.bind();
    m_biTangentBuffer.setUsagePattern(usage);
    fillData(m_biTangent, vertexCount, {0.0f, 1.0f, 0.0f});
    m_biTangentBuffer.allocate(m_biTangent.data(), int(m_biTangent.size() * sizeof(float)));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(3);

    // 若为STATIC或FEM类型绑定colorBuffer
    /* ===== Color Buffer ===== */
    /* ====== location = 4 =====*/
    if (m_objectType == ObjectType::FEM || m_objectType == ObjectType::STATIC)
    {
        m_colorBuffer.bind();
        QOpenGLBuffer::UsagePattern colorUsage = (m_objectType == ObjectType::FEM) ? QOpenGLBuffer::DynamicDraw : QOpenGLBuffer::StaticDraw;
        m_colorBuffer.setUsagePattern(colorUsage);
        fillData(m_colorBufferData, vertexCount, {0.0f, 0.0f, 0.0f}); // 补全颜色信息，若空使用黑色填充
        m_colorBuffer.allocate(m_colorBufferData.data(), int(m_colorBufferData.size() * sizeof(float)));
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(4);
    }

    // 若为MATERIAL类型，上传TexCoord
    /* ===== TexCoord Buffer ===== */
    /* ======== location = 5 ======*/
    if (m_objectType == ObjectType::MATERIAL)
    {
        m_texCoordBuffer.bind();
        m_texCoordBuffer.setUsagePattern(usage);
        m_texCoordBuffer.allocate(m_texCoord.data(), int(m_texCoord.size() * sizeof(float)));
        glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(5);
    }

    m_vao.release();
}

void Object::updateBuffer()
{
    // 在对应Buffer上使用glBufferSubData之类更新
    // 在QOpenGLBuffer里对应是 write() 或 allocate() 一部分
    // 根据 m_objectType 判断更新逻辑
    m_vao.bind();

    if (m_objectType == ObjectType::FEM)
    {
        // 仅更新 color
        if (m_colorBuffer.isCreated() && !m_colorBufferData.empty())
        {
            m_colorBuffer.bind();
            m_colorBuffer.write(0, m_colorBufferData.data(), qint64(m_colorBufferData.size() * sizeof(float)));
        }
    }
    // STATIC 不更新

    m_vao.release();
}

// ---------------------------------------------------------
// draw() & internal drawWireframe/drawFill
// ---------------------------------------------------------
void Object::draw()
{
    // 如果尚未初始化，则需上传buffer
    if (!m_vao.isCreated())
    {
        // create() done in constructor, but maybe not allocated
        uploadToBuffer();
    }

    // 如果为FEM，可能需要每帧更新颜色
    if (m_objectType == ObjectType::FEM)
    {
        updateBuffer();
    }

    if (!m_shader)
    {
        qWarning() << "[Object::draw] No shader attached!";
        return;
    }
    m_shader->bind();

    // 设置模型矩阵给Shader
    auto model = getModelMatrix();
    m_shader->setUniformValue("uModel", model);

    if (m_objectType == ObjectType::FEM || m_objectType == ObjectType::STATIC)
    {
        m_shader->setUniformValue("uUseMaterial", false);
    }
    else if (m_objectType == ObjectType::MATERIAL)
    {
        m_shader->setUniformValue("uUseMaterial", true);
    }

    // 绑定VAO
    m_vao.bind();

    if (m_objectType == ObjectType::MATERIAL)
    {
        m_material->bindUniforms(m_shader);
    }

    // 根据drawMode
    switch (m_drawMode)
    {
    case DrawMode::WIREFRAME:
        drawWireframe();
        break;
    case DrawMode::FILL:
        drawFill();
        break;
    case DrawMode::WIREFRAME_AND_FILL:
        drawWireframe();
        drawFill();
        break;
    }

    m_vao.release();
}

void Object::drawWireframe()
{
    // 在Qt中仍可使用 glPolygonMode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.0f);
    glEnable(GL_LINE_SMOOTH);

    // m_indices.size()是索引数
    glDrawElements(GL_TRIANGLES, GLsizei(m_indices.size()), GL_UNSIGNED_INT, nullptr);

    // 恢复默认
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Object::drawFill()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, GLsizei(m_indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Object::loadCube()
{
    // 1. 清空旧数据
    m_vertices.clear();
    m_indices.clear();
    m_normals.clear();
    m_tangent.clear();
    m_biTangent.clear();
    m_texCoord.clear();

    // 2. 定义每个面的顶点数据
    //    为了让每个面都能完整使用 [0,1]x[0,1] 的纹理坐标，这里不共享顶点。
    //    每个面 4 个顶点、2 个三角形 (6 个 index)；立方体共 6 个面 → 共 24 个顶点，36 个 index。

    // 顶点顺序的约定：让每个面的四个顶点在视觉上逆时针 (CCW) 排列，以保证法线朝外。
    // 比如 +X 面，朝着 +X 方向看时，顺序为“左下 → 左上 → 右上 → 右下”，这样法线是 (1,0,0)。

    // -------------------- Face +X --------------------
    {
        // Normal (1,0,0), Tangent (0,1,0), Bitangent(0,0,1)
        // 这里的 (0,1,0) 与 (0,0,1) 的选择只是一个参考，保证右手坐标系和 UV 对应即可。
        QVector3D normal(1.0f, 0.0f, 0.0f);
        QVector3D tangent(0.0f, 1.0f, 0.0f);
        QVector3D bitangent(0.0f, 0.0f, 1.0f);

        // 四个顶点：从“左下”到“左上”到“右上”到“右下”
        // 由于面在 +X 侧，因此 x=+0.5 固定；y、z 在 [-0.5, 0.5] 内。
        // UV 的分配让左下 -> uv(0,0), 左上 -> uv(0,1), 右上 -> uv(1,1), 右下 -> uv(1,0)
        // （当然，你也可以按其他方式给四顶点分配 UV，只要保证纹理映射一致就行）
        struct VertexData {
            QVector3D pos;
            QVector2D uv;
        } faceXPos[4] = {
            { QVector3D( 0.5f, -0.5f, -0.5f), QVector2D(0.0f, 0.0f) }, // left-bottom
            { QVector3D( 0.5f,  0.5f, -0.5f), QVector2D(0.0f, 1.0f) }, // left-top
            { QVector3D( 0.5f,  0.5f,  0.5f), QVector2D(1.0f, 1.0f) }, // right-top
            { QVector3D( 0.5f, -0.5f,  0.5f), QVector2D(1.0f, 0.0f) }  // right-bottom
        };

        // 将数据写入 m_vertices, m_normals, m_tangent, m_biTangent, m_texCoord
        for (int i = 0; i < 4; ++i) {
            m_vertices.push_back(faceXPos[i].pos.x());
            m_vertices.push_back(faceXPos[i].pos.y());
            m_vertices.push_back(faceXPos[i].pos.z());

            m_normals.push_back(normal.x());
            m_normals.push_back(normal.y());
            m_normals.push_back(normal.z());

            m_tangent.push_back(tangent.x());
            m_tangent.push_back(tangent.y());
            m_tangent.push_back(tangent.z());

            m_biTangent.push_back(bitangent.x());
            m_biTangent.push_back(bitangent.y());
            m_biTangent.push_back(bitangent.z());

            m_texCoord.push_back(faceXPos[i].uv.x());
            m_texCoord.push_back(faceXPos[i].uv.y());
        }
    }

    // -------------------- Face -X --------------------
    {
        QVector3D normal(-1.0f, 0.0f, 0.0f);
        QVector3D tangent(0.0f, 1.0f, 0.0f);
        // normal x tangent = bitangent
        QVector3D bitangent = QVector3D::crossProduct(normal, tangent); // = (0,0,-1)

        struct VertexData {
            QVector3D pos;
            QVector2D uv;
        } faceXNeg[4] = {
            { QVector3D(-0.5f, -0.5f,  0.5f), QVector2D(0.0f, 0.0f) }, // left-bottom
            { QVector3D(-0.5f,  0.5f,  0.5f), QVector2D(0.0f, 1.0f) }, // left-top
            { QVector3D(-0.5f,  0.5f, -0.5f), QVector2D(1.0f, 1.0f) }, // right-top
            { QVector3D(-0.5f, -0.5f, -0.5f), QVector2D(1.0f, 0.0f) }  // right-bottom
        };

        for (int i = 0; i < 4; ++i) {
            m_vertices.push_back(faceXNeg[i].pos.x());
            m_vertices.push_back(faceXNeg[i].pos.y());
            m_vertices.push_back(faceXNeg[i].pos.z());

            m_normals.push_back(normal.x());
            m_normals.push_back(normal.y());
            m_normals.push_back(normal.z());

            m_tangent.push_back(tangent.x());
            m_tangent.push_back(tangent.y());
            m_tangent.push_back(tangent.z());

            m_biTangent.push_back(bitangent.x());
            m_biTangent.push_back(bitangent.y());
            m_biTangent.push_back(bitangent.z());

            m_texCoord.push_back(faceXNeg[i].uv.x());
            m_texCoord.push_back(faceXNeg[i].uv.y());
        }
    }

    // -------------------- Face +Y --------------------
    {
        QVector3D normal(0.0f, 1.0f, 0.0f);
        QVector3D tangent(1.0f, 0.0f, 0.0f);
        QVector3D bitangent = QVector3D::crossProduct(normal, tangent); // = (0,0,1)

        struct VertexData {
            QVector3D pos;
            QVector2D uv;
        } faceYPos[4] = {
            { QVector3D(-0.5f, 0.5f, -0.5f), QVector2D(0.0f, 0.0f) }, // left-bottom
            { QVector3D(-0.5f, 0.5f,  0.5f), QVector2D(0.0f, 1.0f) }, // left-top
            { QVector3D( 0.5f, 0.5f,  0.5f), QVector2D(1.0f, 1.0f) }, // right-top
            { QVector3D( 0.5f, 0.5f, -0.5f), QVector2D(1.0f, 0.0f) }  // right-bottom
        };

        for (int i = 0; i < 4; ++i) {
            m_vertices.push_back(faceYPos[i].pos.x());
            m_vertices.push_back(faceYPos[i].pos.y());
            m_vertices.push_back(faceYPos[i].pos.z());

            m_normals.push_back(normal.x());
            m_normals.push_back(normal.y());
            m_normals.push_back(normal.z());

            m_tangent.push_back(tangent.x());
            m_tangent.push_back(tangent.y());
            m_tangent.push_back(tangent.z());

            m_biTangent.push_back(bitangent.x());
            m_biTangent.push_back(bitangent.y());
            m_biTangent.push_back(bitangent.z());

            m_texCoord.push_back(faceYPos[i].uv.x());
            m_texCoord.push_back(faceYPos[i].uv.y());
        }
    }

    // -------------------- Face -Y --------------------
    {
        QVector3D normal(0.0f, -1.0f, 0.0f);
        QVector3D tangent(1.0f, 0.0f, 0.0f);
        QVector3D bitangent = QVector3D::crossProduct(normal, tangent); // = (0,0,-1)

        struct VertexData {
            QVector3D pos;
            QVector2D uv;
        } faceYNeg[4] = {
            { QVector3D(-0.5f, -0.5f,  0.5f), QVector2D(0.0f, 0.0f) }, // left-bottom
            { QVector3D(-0.5f, -0.5f, -0.5f), QVector2D(0.0f, 1.0f) }, // left-top
            { QVector3D( 0.5f, -0.5f, -0.5f), QVector2D(1.0f, 1.0f) }, // right-top
            { QVector3D( 0.5f, -0.5f,  0.5f), QVector2D(1.0f, 0.0f) }  // right-bottom
        };

        for (int i = 0; i < 4; ++i) {
            m_vertices.push_back(faceYNeg[i].pos.x());
            m_vertices.push_back(faceYNeg[i].pos.y());
            m_vertices.push_back(faceYNeg[i].pos.z());

            m_normals.push_back(normal.x());
            m_normals.push_back(normal.y());
            m_normals.push_back(normal.z());

            m_tangent.push_back(tangent.x());
            m_tangent.push_back(tangent.y());
            m_tangent.push_back(tangent.z());

            m_biTangent.push_back(bitangent.x());
            m_biTangent.push_back(bitangent.y());
            m_biTangent.push_back(bitangent.z());

            m_texCoord.push_back(faceYNeg[i].uv.x());
            m_texCoord.push_back(faceYNeg[i].uv.y());
        }
    }

    // -------------------- Face +Z --------------------
    {
        QVector3D normal(0.0f, 0.0f, 1.0f);
        QVector3D tangent(1.0f, 0.0f, 0.0f);
        QVector3D bitangent = QVector3D::crossProduct(normal, tangent); // = (0,1,0)

        struct VertexData {
            QVector3D pos;
            QVector2D uv;
        } faceZPos[4] = {
            { QVector3D(-0.5f, -0.5f, 0.5f), QVector2D(0.0f, 0.0f) }, // left-bottom
            { QVector3D( 0.5f, -0.5f, 0.5f), QVector2D(1.0f, 0.0f) }, // right-bottom
            { QVector3D( 0.5f,  0.5f, 0.5f), QVector2D(1.0f, 1.0f) }, // right-top
            { QVector3D(-0.5f,  0.5f, 0.5f), QVector2D(0.0f, 1.0f) }  // left-top
        };

        for (int i = 0; i < 4; ++i) {
            m_vertices.push_back(faceZPos[i].pos.x());
            m_vertices.push_back(faceZPos[i].pos.y());
            m_vertices.push_back(faceZPos[i].pos.z());

            m_normals.push_back(normal.x());
            m_normals.push_back(normal.y());
            m_normals.push_back(normal.z());

            m_tangent.push_back(tangent.x());
            m_tangent.push_back(tangent.y());
            m_tangent.push_back(tangent.z());

            m_biTangent.push_back(bitangent.x());
            m_biTangent.push_back(bitangent.y());
            m_biTangent.push_back(bitangent.z());

            m_texCoord.push_back(faceZPos[i].uv.x());
            m_texCoord.push_back(faceZPos[i].uv.y());
        }
    }

    // -------------------- Face -Z --------------------
    {
        QVector3D normal(0.0f, 0.0f, -1.0f);
        QVector3D tangent(1.0f, 0.0f, 0.0f);
        QVector3D bitangent = QVector3D::crossProduct(normal, tangent); // = (0,-1,0)

        struct VertexData {
            QVector3D pos;
            QVector2D uv;
        } faceZNeg[4] = {
            { QVector3D( 0.5f, -0.5f, -0.5f), QVector2D(1.0f, 0.0f) }, // right-bottom
            { QVector3D(-0.5f, -0.5f, -0.5f), QVector2D(0.0f, 0.0f) }, // left-bottom
            { QVector3D(-0.5f,  0.5f, -0.5f), QVector2D(0.0f, 1.0f) }, // left-top
            { QVector3D( 0.5f,  0.5f, -0.5f), QVector2D(1.0f, 1.0f) }  // right-top
        };

        for (int i = 0; i < 4; ++i) {
            m_vertices.push_back(faceZNeg[i].pos.x());
            m_vertices.push_back(faceZNeg[i].pos.y());
            m_vertices.push_back(faceZNeg[i].pos.z());

            m_normals.push_back(normal.x());
            m_normals.push_back(normal.y());
            m_normals.push_back(normal.z());

            m_tangent.push_back(tangent.x());
            m_tangent.push_back(tangent.y());
            m_tangent.push_back(tangent.z());

            m_biTangent.push_back(bitangent.x());
            m_biTangent.push_back(bitangent.y());
            m_biTangent.push_back(bitangent.z());

            m_texCoord.push_back(faceZNeg[i].uv.x());
            m_texCoord.push_back(faceZNeg[i].uv.y());
        }
    }

    // 3. 准备 Index
    //   按照“每个面 4 个顶点、2 个三角形”进行索引。
    //   face i 对应的顶点起始下标为 offset = i*4。
    //   Triangles = (offset, offset+1, offset+2), (offset, offset+2, offset+3)
    const int faceCount = 6;
    for (int i = 0; i < faceCount; ++i)
    {
        unsigned int offset = i * 4;
        // 第一个三角形
        m_indices.push_back(offset + 0);
        m_indices.push_back(offset + 1);
        m_indices.push_back(offset + 2);
        // 第二个三角形
        m_indices.push_back(offset + 0);
        m_indices.push_back(offset + 2);
        m_indices.push_back(offset + 3);
    }
}

