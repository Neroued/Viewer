#include <memory>
#include <string>
#include <vector>

#include <Camera.h>
#include <Object.h>
#include <Scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

Scene::Scene()
    : m_sceneName("Default Scene Name")
{
    m_backgroundShader.loadFromFile("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
}

Scene::Scene(const std::string &name)
    : m_sceneName(name)
{}

Scene::~Scene() {}

void Scene::draw()
{
    for (auto &obj : m_Objects) {
        auto shader = obj->getShader();
        shader->use();
        auto view = m_camera.getViewMatrix();
        auto projection = m_camera.getProjectionMatrix();

        shader->setUniform("uView", view);
        shader->setUniform("uProjection", projection);

        obj->draw();
    }
}

void Scene::addObject(std::shared_ptr<Object> obj)
{
    m_Objects.push_back(obj);
}

void Scene::addController(std::shared_ptr<ObjectController> ctrl)
{
    m_controllers.push_back(ctrl);
}

void Scene::updateObjects(double dt)
{
    for (auto &ctrl : m_controllers) {
        ctrl->update(dt);
    }
}

void Scene::drawBackgroundAndGround(const glm::vec4 &skyColor, const glm::vec3 &groundColor)
{
    m_backgroundShader.use();
    m_backgroundShader.setUniform("drawmode", 1);

    // 绘制纯色背景作为天空
    glClearColor(skyColor.r, skyColor.g, skyColor.b, skyColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 定义地面模型矩阵
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
    model = glm::scale(model, glm::vec3(50.0f, 1.0f, 50.0f));
    m_backgroundShader.setUniform("uModel", model);

    auto view = m_camera.getViewMatrix();
    auto projection = m_camera.getProjectionMatrix();

    m_backgroundShader.setUniform("uView", view);
    m_backgroundShader.setUniform("uProjection", projection);

    // 定义地面顶点数据
    float groundVertices[] = {
        -1.0f,
        0.0f,
        -1.0f, // 左下角
        1.0f,
        0.0f,
        -1.0f, // 右下角
        1.0f,
        0.0f,
        1.0f, // 右上角

        -1.0f,
        0.0f,
        -1.0f, // 左下角
        1.0f,
        0.0f,
        1.0f, // 右上角
        -1.0f,
        0.0f,
        1.0f // 左上角
    };

    // 定义地面颜色
    float groundColors[] = {groundColor.r,
                            groundColor.g,
                            groundColor.b,
                            groundColor.r,
                            groundColor.g,
                            groundColor.b,
                            groundColor.r,
                            groundColor.g,
                            groundColor.b,

                            groundColor.r,
                            groundColor.g,
                            groundColor.b,
                            groundColor.r,
                            groundColor.g,
                            groundColor.b,
                            groundColor.r,
                            groundColor.g,
                            groundColor.b};

    GLuint VAO, VBO, CBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &CBO);

    // 设置 VAO
    glBindVertexArray(VAO);

    // 顶点缓冲
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // 颜色缓冲
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundColors), groundColors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);

    // 绘制地面
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 清理
    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &CBO);
    glDeleteVertexArrays(1, &VAO);
}
