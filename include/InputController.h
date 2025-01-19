#pragma once

#include <QObject>
#include <QMap>
#include <QPointF>
#include <QVector3D>
#include "Camera.h"


class InputController : public QObject
{
    Q_OBJECT
public:
    explicit InputController(QObject* parent = nullptr);
    explicit InputController(Camera* camera, QObject* parent = nullptr);

    // 设置相机
    void setCamera(Camera* camera);

    // 速度/灵敏度/InvertY
    void setMoveSpeed(float speed);
    void setRotateSpeed(float speed);
    void setZoomSpeed(float speed);
    void setInvertY(bool invertY);

    // 更新逻辑（每帧/定时器回调时调用）
    void update(float deltaTime);

    // 重置输入状态
    void reset();

    // 如果你希望直接响应Qt事件：
    //   - keyPressEvent, keyReleaseEvent, mousePressEvent, mouseReleaseEvent,
    //     mouseMoveEvent, wheelEvent
    // 可以在一个QWidget或QOpenGLWidget里调用这些public函数。
    // 或者继承InputController并重写.

    // 对应原先的 onKey, onMouseButton, onMouseMove, onScroll
    // 这里可写更贴近 Qt 的参数
    void handleKeyPress(Qt::Key key);
    void handleKeyRelease(Qt::Key key);

    void handleMousePress(Qt::MouseButton button, const QPointF &pos);
    void handleMouseRelease(Qt::MouseButton button, const QPointF &pos);
    void handleMouseMove(const QPointF &pos);
    void handleWheel(float delta); // delta >0 往上滚, <0 往下滚

private:
    // 相机指针
    Camera* m_camera;

    // 移动/旋转/缩放 速度
    float m_moveSpeed;
    float m_rotateSpeed;
    float m_zoomSpeed;

    // 是否反转 Y 轴
    bool m_invertY;

    // 记录按键是否被按下
    QMap<Qt::Key, bool> m_keyPressed;

    // 鼠标状态
    bool m_firstMouse;
    QPointF m_lastPos;  
    bool m_leftButtonPressed;
    bool m_rightButtonPressed;

private:
    // 处理按键对应的相机移动
    void handleKeyboardMovement(float deltaTime);
};
