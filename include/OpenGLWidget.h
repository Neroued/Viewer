#pragma once

/* Application -- Scene -- Object 架构的最顶层
 * 管理窗口指针和场景
 * 功能尚且不完善
 * 迁移到使用Qt
 */

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <vector>
#include <memory>
#include <string>

#include <Scene.h>
#include <InputController.h>
#include <ShaderManager.h>

class Scene;
class InputController;

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();

    void addScene(std::shared_ptr<Scene> scene) { m_Scenes.push_back(scene); }
    void changeScene(int k);
    void initializeScenes(); // 在添加场景后，再将场景初始化

protected:
    void initializeGL() override;                  // Qt初始化OpenGL
    void resizeGL(int width, int height) override; // 窗口大小变化时调用
    void paintGL() override;                       // 每次绘制时调用

private:
    std::string m_title;
    float m_width, m_height;

    std::vector<std::shared_ptr<Scene>> m_Scenes;
    int currentScene;

    InputController *m_inputController;

public:
    ShaderManager *m_shaderManager;

private:
    // -------------------
    // 事件处理：替代bindInputController + GLFW回调
    // -------------------
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void setInputControllerCamera();
    void updateScene(float dt);
};