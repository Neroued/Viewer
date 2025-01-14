#pragma once

/* Object渲染对象
 * 是渲染架构中最小的组成单元
 * 通过DrawMode和ObjectType来区分不同对象类型，进行不同的渲染方式
 * 需要设置好对应的shader
 * draw()方法需要在上一级Scene提供的语境中使用，即需要设置好相机相关的数据
 */

#include <QOpenGLFunctions> // 替换GLAD/GLFW
#include <QVector3D>        // 替换glm::vec3
#include <QQuaternion>      // 替换glm::quat
#include <QMatrix4x4>       // 替换glm::mat4
#include <QOpenGLBuffer>    // 可以使用QOpenGLBuffer/QOpenGLVertexArrayObject等
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QString>
#include <vector>

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
    STATIC,     // 静态对象，数据在Object创建时上传到GPU
    DYNAMIC,    // 动态对象，每一帧重新上传数据到GPU
    SEMI_STATIC // 半静态对象，顶点位置在创建时上传，颜色数据动态上传
};

class Object : public QOpenGLFunctions
{
public:
    Object();
    ~Object();

    friend ObjectController; // 声明为友元，允许对应的controller直接修改内部数据

    std::size_t getVerticesSize() const { return m_vertices.size(); }
    std::size_t getIndicesSize() const { return m_indices.size(); }

    // 变换设置
    void setPosition(const QVector3D &pos);
    void setRotation(const QQuaternion &rot);
    void setScale(const QVector3D &scl);
    QMatrix4x4 getModelMatrix();

    void loadFromMesh(const Mesh &mesh);
    bool loadFromGLB(const QString &filePath);

    void setShader(const QString &shaderName);
    QOpenGLShaderProgram *getShader() const { return m_shader; }
    void setColorBuffer(const std::vector<float> &colorData);

    void setDrawMode(DrawMode mode);
    DrawMode getDrawMode() const;
    void setObjectType(ObjectType type);
    ObjectType getObjectType() const;

    void initialize(); // 在添加到Scene后由该scene调用
    void draw();       // 需要在上一级Scene提供的语境中使用

private:
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_indexBuffer;
    QOpenGLBuffer m_normalBuffer;
    QOpenGLBuffer m_colorBuffer;

    std::vector<float> m_vertices;       // 每三个元素表示一个点的坐标
    std::vector<unsigned int> m_indices; // 每三个元素表示一个三角形的三个顶点
    std::vector<float> m_normals;
    std::vector<float> m_colorBufferData;

    QVector3D m_position;
    QQuaternion m_rotation;
    QVector3D m_scale;
    QMatrix4x4 m_modelMatrix;
    bool m_shouldUpdateModelMatrix;

    QString m_shaderName;
    QOpenGLShaderProgram *m_shader; // 使用的shader，可与其他对象共享
    DrawMode m_drawMode;
    ObjectType m_objectType;

    bool m_initialized = false; // 是否已初始化
private:
    void uploadToBuffer();
    void updateBuffer();

    void drawWireframe();
    void drawFill();

    void updateModelMatrix();
};
