#include "OpenGLWidget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QElapsedTimer>
#include <QDateTime>
#include <QTimer>
#include <QDebug>

#include <cmath>

#include <Camera.h>
#include <InputController.h>
#include <Scene.h>

#include <Mesh.h>

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), m_title("Default Title"), m_width(800), m_height(450), m_currentScene(nullptr), m_shaderManager(nullptr)
{
    // 如果需要持续刷新(动画)，可使用QTimer定时触发update():
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]()
            {
                update(); // 这会触发 paintGL()
            });
    timer->start(4); // 限制触发帧率

    m_inputController = new InputController(this); // 创建inputController
    setFocusPolicy(Qt::StrongFocus);               // 设置焦点，接受键盘输入
}

OpenGLWidget::~OpenGLWidget()
{
}

void OpenGLWidget::addScene(QSharedPointer<Scene> &scene)
{
    if (scene && !m_scenes.contains(scene->m_sceneName))
    {
        m_scenes.insert(scene->m_sceneName, scene);

        // 若为首个插入的scene
        if (!m_currentScene)
        {
            m_currentScene = scene;
        }
    }
    else
    {
        qWarning() << "Warning: Invalid Scene: nullptr or already exists.";
    }
}

void OpenGLWidget::changeScene(const QString &name)
{
    // TODO: 需要在切换场景前停止上一个场景的一些计算
    auto it = m_scenes.find(name);
    if (it != m_scenes.end())
    {
        m_currentScene = it.value();
        setInputControllerCamera();
    }
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST); // 启用深度测试
    // glEnable(GL_CULL_FACE); // 背面剔除

    // 初次调用，创建ShaderManager
    // 在这里加载所需要的所有shader
    m_shaderManager = ShaderManager::instance();

    m_shaderManager->loadShader("basic", ":/shaders/basic_vertex.glsl", ":/shaders/basic_fragment.glsl");
    m_shaderManager->loadShader("blinn_phong", ":/shaders/blinn_phong_vertex.glsl", ":/shaders/blinn_phong_fragment.glsl");

    m_timer.start(); // 初始化计时器
}

void OpenGLWidget::resizeGL(int w, int h)
{
    m_width = static_cast<float>(w);
    m_height = static_cast<float>(h);

    glViewport(0, 0, w, h);

    if (m_currentScene)
    {
        m_currentScene->m_camera.setAspect(m_width / m_height);
    }
}

void OpenGLWidget::paintGL()
{
    // 1) 清屏
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2) 计算dt
    static qint64 lastTimeMs = 0;
    qint64 currentTimeMs = QDateTime::currentMSecsSinceEpoch();
    float dt = float(currentTimeMs - lastTimeMs) / 1000.0f;
    lastTimeMs = currentTimeMs;

    if (dt > 0.1f)
    {
        dt = 0.016f; // 避免暂停后dt过大
    }

    // 3) 更新输入 & 场景
    m_inputController->update(dt);
    updateScene(dt);

    // 4) 绘制当前场景
    if (m_currentScene)
    {
        // 如果你有类似drawBackgroundAndGround()可在这里调用
        QVector4D skyColor(0.529f, 0.808f, 0.980f, 1.0f); // 天蓝色
        QVector3D groundColor(0.75f, 0.75f, 0.75f);       // 淡灰色
        m_currentScene->drawBackgroundAndGround(skyColor, groundColor);
        m_currentScene->draw();
    }

    // FPS calculation
    m_frameCount++;
    qint64 elapsed = m_timer.elapsed();
    if (elapsed >= 1000) // Calculate FPS every second
    {
        float fps = m_frameCount * 1000.0f / elapsed;
        emit fpsUpdated(fps); // Emit the FPS update signal
        m_frameCount = 0;     // Reset the frame count
        m_timer.restart();    // Restart the timer
    }
}

void OpenGLWidget::setInputControllerCamera()
{
    if (m_scenes.isEmpty())
    {
        qWarning() << "Error: No Scene Exists.";
        return;
    }

    if (!m_currentScene)
    {
        qWarning() << "Error: No Current Scene.";
        return;
    }

    // 把当前场景的相机交给InputController
    Camera &cam = m_currentScene->m_camera;
    m_inputController->setCamera(&cam);
    m_inputController->reset();
    cam.setAspect(m_width / m_height);
}

void OpenGLWidget::initializeScenes()
{
    for (auto &scene : m_scenes)
    {
        scene->initialize();
    }

    setInputControllerCamera();
}

void OpenGLWidget::updateScene(float dt)
{
    if (!m_scenes.isEmpty() && m_currentScene)
    {
        m_currentScene->updateObjects(dt);
    }
}

// --------------------------- 事件处理 ---------------------------
void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    // 如果是自动重复事件，可根据需求决定是否忽略
    if (!event->isAutoRepeat())
    {
        // 调用 handleKeyPress
        m_inputController->handleKeyPress(Qt::Key(event->key()));
    }
    QOpenGLWidget::keyPressEvent(event);
}

void OpenGLWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (!event->isAutoRepeat())
    {
        // 调用 handleKeyRelease
        m_inputController->handleKeyRelease(Qt::Key(event->key()));
    }
    QOpenGLWidget::keyReleaseEvent(event);
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    // 直接调用 InputController 的 handleMousePress
    m_inputController->handleMousePress(event->button(), event->pos());

    QOpenGLWidget::mousePressEvent(event);
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_inputController->handleMouseRelease(event->button(), event->pos());

    QOpenGLWidget::mouseReleaseEvent(event);
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_inputController->handleMouseMove(event->pos());

    QOpenGLWidget::mouseMoveEvent(event);
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
    // 在Qt中滚轮有像素/角度等模式。这里使用angleDelta().y() / 120
    float delta = float(event->angleDelta().y()) / 120.0f;
    m_inputController->handleWheel(delta);

    QOpenGLWidget::wheelEvent(event);
}
