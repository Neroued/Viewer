#include "MatrixVisualizer.h"
#include <QDebug>
#include <QImage>
#include <QSurfaceFormat>

MatrixVisualizer::MatrixVisualizer(QWidget *parent)
    : QOpenGLWidget(parent),
      m_numRows(0),
      m_numCols(0),
      m_program(nullptr)
{
}

MatrixVisualizer::~MatrixVisualizer()
{
}

void MatrixVisualizer::initializeGL()
{
    // 初始化 OpenGL 函数 (3.3 Core)
    initializeOpenGLFunctions();

    // 创建 & 编译着色器程序
    m_program = new QOpenGLShaderProgram(this);

    // 顶点着色器 (Vertex Shader)
    static const char *vsSource =
        R"(#version 330 core
        layout(location = 0) in vec2 a_position;

        uniform mat4 u_transform;

        void main()
        {
            // 将二维坐标乘以正交投影矩阵变换到裁剪坐标
            gl_Position = u_transform * vec4(a_position, 0.0, 1.0);
            // 每个点的大小，可根据需求修改
            gl_PointSize = 3.0;
        })";

    // 片段着色器 (Fragment Shader)
    static const char *fsSource =
        R"(#version 330 core
        out vec4 fragColor;

        void main()
        {
            // 简单地将点画成黑色
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        })";

    // 加载并编译
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vsSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fsSource);
    m_program->link();

    // 创建 VAO
    m_vao.create();
    // 创建 VBO
    m_vbo.create();

    // 绑定 VAO
    m_vao.bind();

    // 绑定 VBO
    m_vbo.bind();
    // 先不上传数据，在 setMatrix() 里上传
    // 只是告诉 OpenGL: 在 location=0 处的顶点数据格式为 (x, y)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                          // layout (location=0)
        2,                          // (x, y)
        GL_FLOAT,                   // 数据类型
        GL_FALSE,                   // 是否归一化
        2 * sizeof(float),          // 每个顶点的步长
        reinterpret_cast<void *>(0) // 在VBO中的偏移
    );

    // 解绑定 VAO
    m_vao.release();

    // 设置背景色（白色）
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void MatrixVisualizer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    updateProjection(w, h);
}

void MatrixVisualizer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 使用我们编写的着色器程序
    m_program->bind();

    // 给着色器传递正交投影矩阵
    m_program->setUniformValue("u_transform", m_projMatrix);

    // 绑定 VAO
    m_vao.bind();

    // 准备绘制
    GLsizei vertexCount = static_cast<GLsizei>(m_colIndices.size());
    // 对于纯点绘制，非零元素个数 = 顶点数

    // 进行绘制 (GL_POINTS 模式)
    glDrawArrays(GL_POINTS, 0, vertexCount);

    // 解绑定
    m_vao.release();
    m_program->release();
}

bool MatrixVisualizer::saveImage(const QString &filePath)
{
    // Qt 提供的抓取当前绘制图像的方法
    QImage image = this->grabFramebuffer();
    return image.save(filePath);
}

void MatrixVisualizer::setMatrix(size_t numRows,
                                 size_t numCols,
                                 const std::vector<size_t> &rowOffsets,
                                 const std::vector<size_t> &colIndices)
{
    m_numRows = numRows;
    m_numCols = numCols;
    m_rowOffsets = rowOffsets;
    m_colIndices = colIndices;

    // 将非零元素 (row, col) 转成 float 顶点数组
    // 这里 row/col 直接作为 2D 坐标
    std::vector<float> vertices;
    vertices.reserve(m_colIndices.size() * 2);

    for (size_t row = 0; row < m_numRows; ++row)
    {
        size_t start = m_rowOffsets[row];
        size_t end = m_rowOffsets[row + 1];
        for (size_t idx = start; idx < end; ++idx)
        {
            size_t col = m_colIndices[idx];
            // x = col, y = row
            vertices.push_back(static_cast<float>(col) / numCols * width());
            vertices.push_back(static_cast<float>(row) / numRows * height());
        }
    }

    // 绑定 VAO
    m_vao.bind();
    // 绑定 VBO
    m_vbo.bind();
    // 上传数据
    m_vbo.allocate(vertices.data(), int(vertices.size() * sizeof(float)));
    glEnableVertexAttribArray(0);

    // 请求重绘
    update();
}

void MatrixVisualizer::updateProjection(int w, int h)
{
    // 重新计算正交投影矩阵
    m_projMatrix.setToIdentity();

    // 仅用于 2D 绘制：左下角(0,0), 右上角(numCols, numRows)
    // 如果希望 row=0 在顶部，则可改为(0, numRows, numCols, 0)等
    m_projMatrix.ortho(
        0.0f, static_cast<float>(width()), // left, right
        0.0f, static_cast<float>(height()), // bottom, top
        -1.0f, 1.0f                          // near, far
    );
}
