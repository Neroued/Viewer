#include "Object.h"
#include <QDebug>

#include "Mesh.h" // 如果需要把 Mesh 解析为顶点索引

Object::Object()
    : m_position(0.0f, 0.0f, 0.0f)
    , m_rotation()
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_shouldUpdateModelMatrix(true)
    , m_shaderManager(nullptr)
    , m_shaderName("basic")
    , m_shader(nullptr)
    , m_drawMode(DrawMode::FILL)
    , m_objectType(ObjectType::SEMI_STATIC)
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

void Object::loadFromMesh(const Mesh &mesh)
{
    m_vertices.resize(mesh.vertices.size * 3);
    m_indices.resize(mesh.indices.size);

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
}

void Object::setShader(const QString &shaderName)
{   
    m_shaderName = shaderName;
    if (m_shaderManager)
    {
        m_shader = m_shaderManager->getShader(shaderName);
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

    // === Color Buffer ===
    m_colorBuffer.bind();
    QOpenGLBuffer::UsagePattern colorUsage = usage;
    // SEMI_STATIC 在这里可视具体情况设置
    if (m_objectType == ObjectType::SEMI_STATIC)
    {
        colorUsage = QOpenGLBuffer::DynamicDraw;
    }
    m_colorBuffer.setUsagePattern(colorUsage);
    if (!m_colorBufferData.empty())
    {
        m_colorBuffer.allocate(m_colorBufferData.data(),
                               int(m_colorBufferData.size() * sizeof(float)));
    }
    else
    {
        // 如果空，则暂时分配一个小空间 or leave it
        float defaultColor[3] = {0.0f, 0.0f, 0.0f};
        m_colorBuffer.allocate(defaultColor, int(sizeof(defaultColor)));
    }
    // 假定每个顶点3个float表示颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

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
