#include "InputController.h"
#include <QDebug>
#include <QtMath> // if needed


// 默认常量
static const float DEFAULT_MOVE_SPEED = 1.5f;
static const float DEFAULT_ROTATE_SPEED = 0.1f;
static const float DEFAULT_ZOOM_SPEED = 1.2f;
static const bool DEFAULT_INVERT_Y = false;

InputController::InputController(QObject *parent)
    : QObject(parent), m_camera(nullptr), m_moveSpeed(DEFAULT_MOVE_SPEED), m_rotateSpeed(DEFAULT_ROTATE_SPEED), m_zoomSpeed(DEFAULT_ZOOM_SPEED), m_invertY(DEFAULT_INVERT_Y), m_firstMouse(true), m_lastPos(0.0, 0.0), m_leftButtonPressed(false), m_rightButtonPressed(false)
{
}

InputController::InputController(Camera *camera, QObject *parent)
    : QObject(parent), m_camera(camera), m_moveSpeed(DEFAULT_MOVE_SPEED), m_rotateSpeed(DEFAULT_ROTATE_SPEED), m_zoomSpeed(DEFAULT_ZOOM_SPEED), m_invertY(DEFAULT_INVERT_Y), m_firstMouse(true), m_lastPos(0.0, 0.0), m_leftButtonPressed(false), m_rightButtonPressed(false)
{
}

void InputController::setCamera(Camera *camera)
{
    if (!camera)
    {
        qWarning() << "[InputController] camera pointer is null!";
        return;
    }
    m_camera = camera;
}

void InputController::setMoveSpeed(float speed)
{
    if (speed <= 0.0f)
    {
        qWarning() << "[InputController] Move speed must be > 0.";
        return;
    }
    m_moveSpeed = speed;
}

void InputController::setRotateSpeed(float speed)
{
    if (speed <= 0.0f)
    {
        qWarning() << "[InputController] Rotate speed must be > 0.";
        return;
    }
    m_rotateSpeed = speed;
}

void InputController::setZoomSpeed(float speed)
{
    if (speed <= 0.0f)
    {
        qWarning() << "[InputController] Zoom speed must be > 0.";
        return;
    }
    m_zoomSpeed = speed;
}

void InputController::setInvertY(bool invertY)
{
    m_invertY = invertY;
}

void InputController::update(float deltaTime)
{
    // 每帧(或每个周期)更新时，处理按键移动
    handleKeyboardMovement(deltaTime);
}

void InputController::reset()
{
    m_keyPressed.clear();
    m_firstMouse = true;
    m_leftButtonPressed = false;
    m_rightButtonPressed = false;
    m_lastPos = QPointF(0.0, 0.0);
}

// ----------- Qt 事件处理对接 -----------

void InputController::handleKeyPress(Qt::Key key)
{
    m_keyPressed[key] = true;
}
void InputController::handleKeyRelease(Qt::Key key)
{
    m_keyPressed[key] = false;
}

void InputController::handleMousePress(Qt::MouseButton button, const QPointF &pos)
{
    if (button == Qt::LeftButton)
    {
        m_leftButtonPressed = true;
    }
    else if (button == Qt::RightButton)
    {
        m_rightButtonPressed = true;
    }
    // 记录当前位置
    m_lastPos = pos;
    m_firstMouse = true;
}

void InputController::handleMouseRelease(Qt::MouseButton button, const QPointF &pos)
{
    if (button == Qt::LeftButton)
    {
        m_leftButtonPressed = false;
    }
    else if (button == Qt::RightButton)
    {
        m_rightButtonPressed = false;
    }
}

void InputController::handleMouseMove(const QPointF &pos)
{
    // 只有在左键按住时才旋转相机 (依据你原逻辑)
    if (!m_leftButtonPressed || !m_camera)
    {
        m_firstMouse = true;
        return;
    }

    if (m_firstMouse)
    {
        // 第一次捕获鼠标或刚按下左键，重置LastPos
        m_lastPos = pos;
        m_firstMouse = false;
        return;
    }

    float xoffset = float(pos.x() - m_lastPos.x());
    float yoffset = float(pos.y() - m_lastPos.y());

    m_lastPos = pos;

    if (m_invertY)
    {
        yoffset = -yoffset;
    }

    // 旋转
    if (m_camera)
    {
        m_camera->rotate(-yoffset * m_rotateSpeed,
                         -xoffset * m_rotateSpeed,
                         0.0f);
    }
}

void InputController::handleWheel(float delta)
{
    // Qt: delta>0 表示向上滚
    // 你原先： camera->zoom( -yoffset * m_zoomSpeed )
    // 这里 delta = yoffset
    if (m_camera)
    {
        m_camera->zoom(-delta * m_zoomSpeed);
    }
}

// ----------- 私有函数: 按键移动相机 -----------
void InputController::handleKeyboardMovement(float deltaTime)
{
    if (!m_camera)
        return;

    // 构建一个 (x,y,z) 记录WSADQE的位移分量
    QVector3D movement(0.0f, 0.0f, 0.0f);

    // W S => 前后
    if (m_keyPressed.value(Qt::Key_W, false))
    {
        movement.setZ(movement.z() + m_moveSpeed * deltaTime);
    }
    if (m_keyPressed.value(Qt::Key_S, false))
    {
        movement.setZ(movement.z() - m_moveSpeed * deltaTime);
    }

    // A D => 左右
    if (m_keyPressed.value(Qt::Key_A, false))
    {
        movement.setX(movement.x() - m_moveSpeed * deltaTime);
    }
    if (m_keyPressed.value(Qt::Key_D, false))
    {
        movement.setX(movement.x() + m_moveSpeed * deltaTime);
    }

    // Q E => 上下
    if (m_keyPressed.value(Qt::Key_Q, false))
    {
        movement.setY(movement.y() + m_moveSpeed * deltaTime);
    }
    if (m_keyPressed.value(Qt::Key_E, false))
    {
        movement.setY(movement.y() - m_moveSpeed * deltaTime);
    }

    // 若 movement 非零，则移动相机
    if (!movement.isNull())
    {
        // 你在Camera中定义了moveForward/Right/Up
        //  z分量 => forward, x分量 => right, y分量 => up
        m_camera->moveForward(movement.z());
        m_camera->moveRight(movement.x());
        m_camera->moveUp(movement.y());
    }
}
