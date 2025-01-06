#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Object.h>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Mesh.h>

Object::Object()
    : position(0.0f, 0.0f, 0.0f)
    , rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
    , scale(1.0f, 1.0f, 1.0f)
    , shouldUpdateModelMatrix(true)
    , drawmode(DrawMode::FILL)
    , objecttype(ObjectType::SEMI_STATIC)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &colorVBO);
}

Object::~Object() = default;

void Object::updateModelMatrix()
{
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMatrix = glm::toMat4(rotation);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
    modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}

glm::mat4 Object::getModelMatrix()
{
    if (shouldUpdateModelMatrix) {
        updateModelMatrix();
        shouldUpdateModelMatrix = false;
    }

    return modelMatrix;
}

void Object::setPosition(const glm::vec3 &pos)
{
    position = pos;
    shouldUpdateModelMatrix = true;
}

void Object::setRotation(const glm::quat &rot)
{
    rotation = rot;
    shouldUpdateModelMatrix = true;
}

void Object::setScale(const glm::vec3 &scl)
{
    scale = scl;
    shouldUpdateModelMatrix = true;
}

void Object::loadFromMesh(const Mesh &mesh)
{
    vertices.resize(mesh.vertices.size * 3);
    indices.resize(mesh.indices.size);

    std::size_t k = 0;
    for (std::size_t i = 0; i < mesh.vertices.size; ++i) {
        vertices[k++] = (float) mesh.vertices[i].x;
        vertices[k++] = (float) mesh.vertices[i].y;
        vertices[k++] = (float) mesh.vertices[i].z;
    }

    for (std::size_t i = 0; i < mesh.indices.size; ++i) {
        indices[i] = mesh.indices[i];
    }

    uploadToBuffer();
}

void Object::setDrawMode(DrawMode mode)
{
    drawmode = mode;
}

DrawMode Object::getDrawMode() const
{
    return drawmode;
}

void Object::setObjectType(ObjectType type)
{
    objecttype = type;
}

ObjectType Object::getObjectType() const
{
    return objecttype;
}

void Object::uploadVertex()
{
    // 上传顶点数据到 VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    GLenum usage = (objecttype == ObjectType::DYNAMIC) ? GL_DYNAMIC_DRAW
                                                       : GL_STATIC_DRAW; // 动态或静态
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), usage);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(0); // 位置属性：location = 0
}

void Object::uploadElement()
{
    // 索引数据绑定 EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    GLenum usage = (objecttype == ObjectType::DYNAMIC) ? GL_DYNAMIC_DRAW
                                                       : GL_STATIC_DRAW; // 动态或静态
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 usage);
}

void Object::uploadColorBuffer()
{
    // 上传颜色数据到 colorVBO
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    if (colorBuffer.empty()) {
        // 如果 colorBuffer 为空，上传一个默认的颜色数据
        float defaultColor[3] = {0.0f, 0.0f, 0.0f};
        glBufferData(GL_ARRAY_BUFFER, sizeof(defaultColor), defaultColor, GL_STATIC_DRAW);
    } else {
        GLenum usage = (objecttype == ObjectType::DYNAMIC || objecttype == ObjectType::SEMI_STATIC)
                           ? GL_DYNAMIC_DRAW
                           : GL_STATIC_DRAW;
        glBufferData(GL_ARRAY_BUFFER, colorBuffer.size() * sizeof(float), colorBuffer.data(), usage);
    }
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(1); // 颜色属性：location = 1
}

void Object::uploadToBuffer()
{
    glBindVertexArray(VAO);

    uploadVertex();
    uploadElement();
    uploadColorBuffer();

    glBindVertexArray(0); // 解绑 VAO
}

void Object::updateVertex()
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
}

void Object::updateElement()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                    0,
                    indices.size() * sizeof(unsigned int),
                    indices.data());
}

void Object::updateColorBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, colorBuffer.size() * sizeof(float), colorBuffer.data());
}

void Object::updateBuffer()
{
    switch (objecttype) {
    case ObjectType::STATIC:
        break;

    case ObjectType::SEMI_STATIC:
        updateColorBuffer();
        break;

    case ObjectType::DYNAMIC:
        updateVertex();
        updateElement();
        updateColorBuffer();
        break;

    default:
        break;
    }
}

void Object::drawWireframe()
{
    shader->setUniform("drawmode", 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.0f);
    glEnable(GL_LINE_SMOOTH);
    glDrawElements(GL_TRIANGLES, getIndicesSize(), GL_UNSIGNED_INT, 0);
}

void Object::drawFill()
{
    shader->setUniform("drawmode", 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, getIndicesSize(), GL_UNSIGNED_INT, 0);
}

void Object::draw()
{
    glm::mat4 model = getModelMatrix();
    shader->setUniform("uModel", model);

    glBindVertexArray(VAO);

    updateBuffer();

    switch (drawmode) {
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

    default:
        break;
    }

    glBindVertexArray(0);
}
