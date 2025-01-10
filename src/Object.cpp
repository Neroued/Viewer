#include "Object.h"
#include <QDebug>
#include <QFile>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "Mesh.h" // 如果需要把 Mesh 解析为顶点索引

Object::Object()
    : m_position(0.0f, 0.0f, 0.0f), m_rotation(), m_scale(1.0f, 1.0f, 1.0f), m_shouldUpdateModelMatrix(true), m_shaderManager(nullptr), m_shaderName("basic"), m_shader(nullptr), m_drawMode(DrawMode::FILL), m_objectType(ObjectType::SEMI_STATIC)
{
}

Object::~Object()
{
}

void Object::initialize()
{
    initializeOpenGLFunctions();

    // Initialize VAO
    if (!m_vao.create())
    {
        qWarning() << "[Object] Failed to create VAO!";
    }

    // Initialize Buffers
    m_vertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vertexBuffer.create();

    m_indexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_indexBuffer.create();

    m_colorBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_colorBuffer.create();

    uploadToBuffer();

    if (m_shaderManager)
    {
        m_shader = m_shaderManager->getShader(m_shaderName);
    }
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
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open GLB file from QRC:" << filePath;
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // 将 QByteArray 转换为 std::vector<unsigned char>
    std::vector<unsigned char> data(fileData.begin(), fileData.end());

    // 使用 TinyGLTF 加载二进制数据
    bool success_load = loader.LoadBinaryFromMemory(&model, &err, &warn, data.data(), data.size());
    if (!success_load) {
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

void Object::setShader(const QString &shaderName)
{

    if (m_shaderManager) // 表示Object已创建，存在shaderManager
    {
        auto s = m_shaderManager->getShader(shaderName);
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
    else // 首次创建，还未绑定shaderManager
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

// ---------------------------------------------------------
// Buffer upload / update
// ---------------------------------------------------------
void Object::uploadToBuffer()
{
    // 绑定VAO
    m_vao.bind();

    // === Vertex Buffer ===
    m_vertexBuffer.bind();
    QOpenGLBuffer::UsagePattern usage = QOpenGLBuffer::StaticDraw;
    if (m_objectType == ObjectType::DYNAMIC)
    {
        usage = QOpenGLBuffer::DynamicDraw;
    }
    m_vertexBuffer.setUsagePattern(usage);
    m_vertexBuffer.allocate(m_vertices.data(), int(m_vertices.size() * sizeof(float)));

    // 顶点属性 (location=0), 3个float
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // === Index Buffer ===
    m_indexBuffer.bind();
    m_indexBuffer.setUsagePattern(usage);
    m_indexBuffer.allocate(m_indices.data(), int(m_indices.size() * sizeof(unsigned int)));
    // Note: QOpenGLBuffer::IndexBuffer is automatically bound to GL_ELEMENT_ARRAY_BUFFER

    // === Normal Buffer ===
    m_normalBuffer.bind();
    m_normalBuffer.setUsagePattern(usage);
    if (!m_normals.empty())
    {
        m_normalBuffer.allocate(m_normals.data(),
                                int(m_normals.size() * sizeof(float)));
    }
    else
    {
        // 如果法向量数据为空，分配一个默认法向量 (0, 0, 1)
        float defaultNormal[3] = {0.0f, 0.0f, 1.0f};
        m_normalBuffer.allocate(defaultNormal, int(sizeof(defaultNormal)));
    }
    // 假定每个顶点3个float表示法向量
    // (location=1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    // === Color Buffer ===
    // (location=2)
    m_colorBuffer.bind();
    QOpenGLBuffer::UsagePattern colorUsage = usage;

    // 根据对象类型设置 UsagePattern
    if (m_objectType == ObjectType::SEMI_STATIC)
    {
        colorUsage = QOpenGLBuffer::DynamicDraw;
    }
    m_colorBuffer.setUsagePattern(colorUsage);

    // 检查颜色缓冲区数据
    if (!m_colorBufferData.empty())
    {
        // 确保颜色数据大小与顶点数量匹配
        size_t vertexCount = m_vertices.size() / 3;       // 顶点数量 (每个顶点 3 个 float)
        size_t colorCount = m_colorBufferData.size() / 3; // 颜色数量 (每个颜色 3 个 float)

        if (colorCount < vertexCount)
        {
            // 获取最后一个颜色
            float lastColor[3] = {
                m_colorBufferData[colorCount * 3 - 3],
                m_colorBufferData[colorCount * 3 - 2],
                m_colorBufferData[colorCount * 3 - 1]};

            // 填充缺少的颜色
            for (size_t i = colorCount; i < vertexCount; ++i)
            {
                m_colorBufferData.push_back(lastColor[0]);
                m_colorBufferData.push_back(lastColor[1]);
                m_colorBufferData.push_back(lastColor[2]);
            }
        }

        m_colorBuffer.allocate(m_colorBufferData.data(),
                               int(m_colorBufferData.size() * sizeof(float)));
    }
    else
    {
        // 如果颜色数据为空，设置默认颜色
        size_t vertexCount = m_vertices.size() / 3;
        QVector<float> defaultColorData(vertexCount * 3, 0.0f); // 默认黑色填充
        m_colorBuffer.allocate(defaultColorData.data(),
                               int(defaultColorData.size() * sizeof(float)));
    }

    // 假定每个顶点 3 个 float 表示颜色
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);

    m_vao.release();
}

void Object::updateBuffer()
{
    // 在对应Buffer上使用glBufferSubData之类更新
    // 在QOpenGLBuffer里对应是 write() 或 allocate() 一部分
    // 根据 m_objectType 判断更新逻辑
    m_vao.bind();

    if (m_objectType == ObjectType::DYNAMIC)
    {
        // 顶点 & 索引 & 颜色都更新
        if (m_vertexBuffer.isCreated())
        {
            m_vertexBuffer.bind();
            m_vertexBuffer.write(0, m_vertices.data(),
                                 qint64(m_vertices.size() * sizeof(float)));
        }
        if (m_indexBuffer.isCreated())
        {
            m_indexBuffer.bind();
            m_indexBuffer.write(0, m_indices.data(),
                                qint64(m_indices.size() * sizeof(unsigned int)));
        }
        if (m_colorBuffer.isCreated() && !m_colorBufferData.empty())
        {
            m_colorBuffer.bind();
            m_colorBuffer.write(0, m_colorBufferData.data(),
                                qint64(m_colorBufferData.size() * sizeof(float)));
        }
    }
    else if (m_objectType == ObjectType::SEMI_STATIC)
    {
        // 仅更新 color
        if (m_colorBuffer.isCreated() && !m_colorBufferData.empty())
        {
            m_colorBuffer.bind();
            m_colorBuffer.write(0, m_colorBufferData.data(),
                                qint64(m_colorBufferData.size() * sizeof(float)));
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

    // 如果DYNAMIC/SEMI_STATIC，可能需要每帧更新
    if (m_objectType == ObjectType::DYNAMIC ||
        m_objectType == ObjectType::SEMI_STATIC)
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

    // 绑定VAO
    m_vao.bind();

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
