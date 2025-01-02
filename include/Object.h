#pragma once
#define GLM_ENABLE_EXPERIMENTAL

/* Object渲染对象
 * 是渲染架构中最小的组成单元
 * 通过DrawMode和ObjectType来区分不同对象类型，进行不同的渲染方式
 * 需要设置好对应的shader
 * draw()方法需要在上一级Scene提供的语境中使用，即需要设置好相机相关的数据
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Shader.h>
#include <memory>

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

class Object
{
public:
    Object();
    ~Object();

    GLuint VAO, VBO, EBO, colorVBO;
    std::vector<float> vertices;       // 每三个元素表示一个点的坐标
    std::vector<unsigned int> indices; // 每三个元素表示一个三角形的三个顶点
    std::vector<float> colorBuffer;

    std::size_t getVerticesSize() const { return vertices.size(); }
    std::size_t getIndicesSize() const { return indices.size(); }

    void setPosition(const glm::vec3 &pos);
    void setRotation(const glm::quat &rot);
    void setScale(const glm::vec3 &scl);
    void updateModelMatrix();
    glm::mat4 getModelMatrix();

    void loadFromMesh(const Mesh &mesh);
    void attachShader(const std::shared_ptr<Shader> &sdr) { shader = sdr; }
    std::shared_ptr<Shader> getShader() const { return shader; }

    void setDrawMode(DrawMode mode);
    DrawMode getDrawMode() const;
    void setObjectType(ObjectType type);
    ObjectType getObjectType() const;

    void draw(); // 需要在上一级Scene提供的语境中使用

private:
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    glm::mat4 modelMatrix;
    bool shouldUpdateModelMatrix;

    std::shared_ptr<Shader> shader; // 使用的shader，可与其他对象共享
    DrawMode drawmode;
    ObjectType objecttype;

private:
    void uploadVertex(); // 辅助函数，均需要在绑定VAO之后使用
    void uploadElement();
    void uploadColorBuffer();
    void uploadToBuffer();

    void updateVertex();
    void updateElement();
    void updateColorBuffer();
    void updateBuffer(); // 更新Buffer中的数据

    void drawWireframe();
    void drawFill();
};
