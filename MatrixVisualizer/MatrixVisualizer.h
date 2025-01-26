#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <vector>

class MatrixVisualizer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit MatrixVisualizer(QWidget *parent = nullptr);
    ~MatrixVisualizer();

    // 设置稀疏矩阵 (CSR) 所需信息
    // 只存储非零位置，不存储 values
    void setMatrix(size_t numRows,
                   size_t numCols,
                   const std::vector<size_t> &rowOffsets,
                   const std::vector<size_t> &colIndices);

    // 保存当前可视化结果到图片文件
    bool saveImage(const QString &filePath);

protected:
    // QOpenGLWidget 必须重写的函数
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    // 1) 记录稀疏矩阵形状 & 非零位置（CSR）
    size_t m_numRows;
    size_t m_numCols;
    std::vector<size_t> m_rowOffsets; // 大小为 numRows+1
    std::vector<size_t> m_colIndices; // 存储所有非零的列索引

    // 2) 用于绘制的 GPU 资源
    QOpenGLShaderProgram *m_program; // 着色器程序
    QOpenGLVertexArrayObject m_vao;  // 顶点数组对象
    QOpenGLBuffer m_vbo;             // 顶点缓冲对象

    // 3) 投影矩阵（2D 正交投影）
    QMatrix4x4 m_projMatrix;

    // 4) 辅助函数
    void setupVertexData();              // 将稀疏矩阵的非零位置转换为顶点数据并上传到 GPU
    void updateProjection(int w, int h); // 计算投影矩阵
};
