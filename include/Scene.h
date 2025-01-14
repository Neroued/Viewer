#pragma once

#include <QString>
#include <QVector>
#include <QSharedPointer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "Camera.h"
#include "Object.h"
#include "ObjectController.h"
#include "ShaderManager.h"


/**
 * @brief 场景类
 *   - 包含相机、对象、控制器
 *   - 负责更新和渲染所有对象
 *   - 可以绘制背景与地面
 */
class Scene : protected QOpenGLFunctions
{
public:
    Scene();
    explicit Scene(const QString &name);
    ~Scene();

    void setSceneName(const QString &name) { m_sceneName = name; }
    QString getSceneName() const { return m_sceneName; }

    void initialize(); // 在添加到mainWindow后调用
    // 绘制整个场景：先绘制每个object
    void draw();

    // 添加对象
    void addObject(const QSharedPointer<Object> &obj);

    // 添加控制器(管理对象的运动/动画等)
    void addController(const QSharedPointer<ObjectController> &ctrl);

    // 更新对象 (控制器)
    void updateObjects(double dt);

    // 获取相机
    Camera &getCamera() { return m_camera; }

    // 绘制背景与地面
    // 用QVector3D/QVector4D替代原先的glm::vec3/vec4
    void drawBackgroundAndGround(const QVector4D &skyColor,
                                 const QVector3D &groundColor);

public:
    QString m_sceneName;
    Camera m_camera; ///< 相机对象(已用Qt方式重构)
private:
    QVector<QSharedPointer<Object>> m_Objects;
    QVector<QSharedPointer<ObjectController>> m_controllers;

    // 绘制背景相关
    void initializeGroundBuffers(const QVector3D &groundColor);
    QOpenGLShaderProgram *m_backgroundShader; 
    QOpenGLVertexArrayObject m_groundVAO;
    QOpenGLBuffer m_groundVBO;
    QOpenGLBuffer m_groundNBO;
    QOpenGLBuffer m_groundCBO;
};
