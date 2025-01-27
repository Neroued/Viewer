#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QMap>
#include <QElapsedTimer>

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

    void addScene(QSharedPointer<Scene> &scene);
    void changeScene(const QString &name);
    void initializeScenes(); // 在添加场景后，再将场景初始化

signals:
    void fpsUpdated(float fps);
    void vertexAndFaceInfoUpdated(const QVector<size_t> info); // 转发信号
    void sceneListUpdated(const QStringList sceneList);

public slots:
    void startCurrentSceneAllControllers() { m_currentScene->startAllController(); }
    void stopCurrentSceneAllControllers() { m_currentScene->stopAllController(); }
    void resetCurrentSceneAllControllers() { m_currentScene->resetAllController(); }
    void onChangeScene(const QString name) { changeScene(name); }

protected:
    void initializeGL() override;                  // 在Widget被显示前第一次调用，且只调用一次
    void resizeGL(int width, int height) override; // 窗口大小变化时调用
    void paintGL() override;                       // 每次绘制时调用

private:
    QString m_title;
    float m_width, m_height;
    QElapsedTimer m_timer; // 用于计时的 QTime
    int m_frameCount;      // 帧计数

    QMap<QString, QSharedPointer<Scene>> m_scenes; // 存储的全部场景
    QSharedPointer<Scene> m_currentScene;          // 当前场景

    InputController *m_inputController; // 使用一个inputController来处理鼠标和键盘事件

    ShaderManager* m_shaderManager; // 在这个类的初始化中创建ShaderManager的instance, 存放指针

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
