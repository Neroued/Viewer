#include "Scene.h"
#include <QDebug>
#include <QMatrix4x4>
#include <QOpenGLExtraFunctions> // 如果需要glPolygonMode或更高级别API
// #include <QOpenGLBuffer>, <QOpenGLVertexArrayObject> 若你想使用Qt封装buffer

#include <iostream>

Scene::Scene()
    : m_sceneName("Default Scene Name"), m_shaderManager(nullptr), m_backgroundShader(nullptr)
{
}

Scene::Scene(const QString &name)
    : m_sceneName(name)
{
}

Scene::~Scene()
{
}

void Scene::initialize()
{
    initializeOpenGLFunctions();

    m_backgroundShader = m_shaderManager->getShader("basic");

    for (auto &obj : m_Objects)
    {
        obj->initialize();
    }

    std::cout << "Scene initialized" << std::endl;
}

void Scene::draw()
{
    // 绘制所有对象
    for (auto &obj : m_Objects)
    {
        auto shader = obj->getShader();
        if (!shader)
        {
            qWarning() << "[Scene::draw] Object has no shader attached!";
            continue;
        }
        shader->bind();

        QMatrix4x4 view = m_camera.getViewMatrix();
        QMatrix4x4 projection = m_camera.getProjectionMatrix();

        shader->setUniformValue("uView", view);
        shader->setUniformValue("uProjection", projection);

        obj->draw();
    }
}

void Scene::addObject(const QSharedPointer<Object> &obj)
{
    m_Objects.push_back(obj);
}

void Scene::addController(const QSharedPointer<ObjectController> &ctrl)
{
    m_controllers.push_back(ctrl);
}

void Scene::updateObjects(double dt)
{
    for (auto &ctrl : m_controllers)
    {
        ctrl->update(dt);
    }
}

void Scene::drawBackgroundAndGround(const QVector4D &skyColor, const QVector3D &groundColor)
{
    // 1) 使用 m_backgroundShader
    m_backgroundShader->bind();

    // 2) 设置清屏色并清屏
    glClearColor(skyColor.x(), skyColor.y(), skyColor.z(), skyColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3) 计算 model/view/projection 矩阵
    QMatrix4x4 model;
    model.translate(0.0f, -2.0f, 0.0f);
    model.scale(50.0f, 1.0f, 50.0f);

    QMatrix4x4 view = m_camera.getViewMatrix();
    QMatrix4x4 projection = m_camera.getProjectionMatrix();

    m_backgroundShader->setUniformValue("uModel", model);
    m_backgroundShader->setUniformValue("uView", view);
    m_backgroundShader->setUniformValue("uProjection", projection);
    m_backgroundShader->setUniformValue("drawmode", 1);

    // 4) 初始化顶点和颜色缓冲（仅初始化一次，避免重复创建）
    if (!m_groundVAO.isCreated())
    {
        initializeGroundBuffers(groundColor);
    }

    // 5) 绑定 VAO 并绘制
    m_groundVAO.bind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_groundVAO.release();

    m_backgroundShader->release();
}

void Scene::setShaderManager(ShaderManager *shaderManager)
{
    m_shaderManager = shaderManager;
}

void Scene::initializeGroundBuffers(const QVector3D &groundColor)
{
    // Ground vertices
    float groundVertices[] = {
        -1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 1.0f,

        -1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 1.0f};

    // Ground colors
    float r = groundColor.x();
    float g = groundColor.y();
    float b = groundColor.z();
    float groundColors[] = {
        r, g, b,
        r, g, b,
        r, g, b,

        r, g, b,
        r, g, b,
        r, g, b};

    // 创建 VAO
    m_groundVAO.create();
    m_groundVAO.bind();

    // 创建 VBO（顶点缓冲）
    m_groundVBO.create();
    m_groundVBO.bind();
    m_groundVBO.allocate(groundVertices, sizeof(groundVertices));

    m_backgroundShader->enableAttributeArray(0); // 位置属性：location = 0
    m_backgroundShader->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    m_groundVBO.release();

    // 创建 CBO（颜色缓冲）
    m_groundCBO.create();
    m_groundCBO.bind();
    m_groundCBO.allocate(groundColors, sizeof(groundColors));

    m_backgroundShader->enableAttributeArray(1); // 颜色属性：location = 1
    m_backgroundShader->setAttributeBuffer(1, GL_FLOAT, 0, 3);

    m_groundCBO.release();
    m_groundVAO.release();
}