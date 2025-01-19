#pragma once

/* Object渲染对象
 * 是渲染架构中最小的组成单元
 * 通过DrawMode和ObjectType来区分不同对象类型，进行不同的渲染方式
 * 需要设置好对应的shader
 * draw()方法需要在上一级Scene提供的语境中使用，即需要设置好相机相关的数据
 */

#include <QObject>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QString>
#include <vector>

#include <Material.h>
#include <ObjectController.h>
#include <ShaderManager.h>
#include <Mesh.h>

enum class DrawMode
{
    WIREFRAME,         // 线框模式
    FILL,              // 填充模式
    WIREFRAME_AND_FILL // 混合模式，绘制Mesh使用
};

enum class ObjectType
{
    STATIC,  // 使用colorBufferData的静态对象
    FEM,     // FEM对象，展示FEM结果，使用colorBufferData
    MATERIAL // 使用Material
};

class Object : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    Object(QObject *parent = nullptr);
    virtual ~Object();

    friend ObjectController; // 声明为友元，允许对应的controller直接修改内部数据

    std::size_t getVerticesSize() const { return m_vertices.size(); }
    std::size_t getIndicesSize() const { return m_indices.size(); }

    // 变换设置
    void setPosition(const QVector3D &pos);
    void setRotation(const QQuaternion &rot);
    void setScale(const QVector3D &scl);
    QMatrix4x4 getModelMatrix();

    void setMaterial(const QString &materialName);
    void setShader(const QString &shaderName);
    QSharedPointer<QOpenGLShaderProgram> getShader() const { return m_shader; }
    void setColorBuffer(const std::vector<float> &colorData);

    void setDrawMode(DrawMode mode);
    DrawMode getDrawMode() const;
    void setObjectType(ObjectType type);
    ObjectType getObjectType() const;

    // 从网格中加载，适用于FEM，自动计算normal, tangent, bitangents
    void loadFromMesh(const FEMLib::Mesh &mesh);

    // 从GLB文件中加载
    // TODO: 一次性加载多个对象，可能需要移动到Scene中，在这里提供相应接口
    bool loadFromGLB(const QString &filePath);

    void initialize(); // 在添加到Scene后由该scene调用
    void draw();       // 需要在上一级Scene提供的语境中使用

    void loadCube(); // 加载一个最简单的立方体

private:
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vertexBuffer;    // 顶点坐标    location=0
    QOpenGLBuffer m_indexBuffer;     // 顶点下标
    QOpenGLBuffer m_normalBuffer;    // 法向量      location=1
    QOpenGLBuffer m_tangentBuffer;   // 切向量      location=2
    QOpenGLBuffer m_biTangentBuffer; // 副切向量    location=3
    QOpenGLBuffer m_colorBuffer;     // 颜色缓冲    location=4
    QOpenGLBuffer m_texCoordBuffer;  // uv坐标      location=5

    std::vector<float> m_vertices;       // 每三个元素表示一个点的坐标
    std::vector<unsigned int> m_indices; // 每三个元素表示一个三角形的三个顶点
    std::vector<float> m_normals;
    std::vector<float> m_tangent;
    std::vector<float> m_biTangent;
    std::vector<float> m_texCoord; // uv坐标，每两个数据对应一个顶点

    std::vector<float> m_colorBufferData; // 颜色数据
    QString m_materialName;
    QSharedPointer<Material> m_material;  // 材质

    QVector3D m_position;
    QQuaternion m_rotation;
    QVector3D m_scale;
    QMatrix4x4 m_modelMatrix;
    bool m_shouldUpdateModelMatrix;

    QString m_shaderName;
    QSharedPointer<QOpenGLShaderProgram> m_shader; // 使用的shader，可与其他对象共享
    DrawMode m_drawMode;
    ObjectType m_objectType;

    bool m_initialized = false; // 是否已初始化
private:
    void createBuffers();
    void uploadToBuffer();
    void updateBuffer();

    void drawWireframe();
    void drawFill();

    void updateModelMatrix();
};
