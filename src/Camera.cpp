#include "Camera.h"
#include <QtMath> // for qDegreesToRadians, qBound
#include <QDebug>

Camera::Camera()
    : m_position(0.0f, 0.0f, 3.0f), m_orientation(1.0f, 0.0f, 0.0f, 0.0f) // identity quaternion
      ,
      m_fov(45.0f), m_aspect(16.0f / 9.0f), m_nearPlane(0.1f), m_farPlane(100.0f), m_minFov(10.0f), m_maxFov(90.0f)
{
}

Camera::Camera(const QVector3D &position,
               const QQuaternion &orientation,
               float fovDeg,
               float aspect,
               float nearPlane,
               float farPlane)
    : m_position(position), m_orientation(orientation.normalized()), m_fov(fovDeg), m_aspect(aspect), m_nearPlane(nearPlane), m_farPlane(farPlane), m_minFov(1.0f), m_maxFov(90.0f)
{
    if (nearPlane <= 0.0f || farPlane <= nearPlane)
    {
        qWarning() << "[Camera] Invalid clipping planes!";
    }
}

void Camera::setPosition(const QVector3D &pos)
{
    m_position = pos;
}

void Camera::setOrientation(const QQuaternion &orientation)
{
    m_orientation = orientation.normalized();
}

void Camera::setOrientation(float pitchDeg, float yawDeg, float rollDeg)
{
    m_orientation = eulerToQuat(pitchDeg, yawDeg, rollDeg);
}

QVector3D Camera::getEulerOrientation() const
{
    return quatToEuler(m_orientation);
}

void Camera::setFOV(float fovDeg)
{
    // 限制在 [m_minFov, m_maxFov] 区间
    m_fov = qBound(m_minFov, fovDeg, m_maxFov);
}

void Camera::setAspect(float aspect)
{
    if (aspect <= 0.0f)
    {
        qWarning() << "[Camera::setAspect] Aspect ratio must be > 0.";
        return;
    }
    m_aspect = aspect;
}

void Camera::setClippingPlanes(float nearPlane, float farPlane)
{
    if (nearPlane <= 0.0f || farPlane <= nearPlane)
    {
        qWarning() << "[Camera::setClippingPlanes] Invalid near/far plane!";
        return;
    }
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

QMatrix4x4 Camera::getViewMatrix() const
{
    QVector3D forward = m_orientation.rotatedVector(QVector3D(0.0f, 0.0f, -1.0f)); // 相机的前方向
    QVector3D up = m_orientation.rotatedVector(QVector3D(0.0f, 1.0f, 0.0f));       // 相机的上方向
    QVector3D center = m_position + forward;                                       // 观察点 = 相机位置 + 前方向

    QMatrix4x4 view;
    view.lookAt(m_position, center, up);

    return view;
}

QMatrix4x4 Camera::getProjectionMatrix() const
{
    // QMatrix4x4::perspective(fovVertical, aspect, nearPlane, farPlane)
    // fovVertical 是角度
    QMatrix4x4 proj;
    proj.perspective(m_fov, m_aspect, m_nearPlane, m_farPlane);
    return proj;
}

void Camera::moveForward(float distance)
{
    // 根据当前视角的前方向移动 (相机的 -Z 方向)
    QVector3D forward = m_orientation.rotatedVector(QVector3D(0.0f, 0.0f, -1.0f));
    m_position += forward * distance;
}

void Camera::moveRight(float distance)
{
    // 根据当前视角的右方向移动 (相机的 +X 方向)
    QVector3D right = m_orientation.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f));
    m_position += right * distance;
}

void Camera::moveUp(float distance)
{
    // 始终沿全局 Y 轴移动 (世界坐标系的 up 方向)
    QVector3D up(0.0f, 1.0f, 0.0f);
    m_position += up * distance;
}

void Camera::rotate(const QVector3D &axis, float angleDeg)
{
    // 根据任意轴旋转
    QQuaternion rotation = QQuaternion::fromAxisAndAngle(axis.normalized(), angleDeg);
    m_orientation = (rotation * m_orientation).normalized();
}

void Camera::rotate(float deltaPitch, float deltaYaw, float deltaRoll)
{
    // 按顺序进行旋转：yaw (世界Y), pitch (相机X), roll (相机Z)
    QVector3D camRight = m_orientation.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f)); // 当前相机的 X 方向

    QQuaternion pitchQ = QQuaternion::fromAxisAndAngle(camRight, deltaPitch);                   // pitch 绕相机 X 轴
    QQuaternion yawQ = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), deltaYaw);    // yaw 绕世界 Y 轴
    QQuaternion rollQ = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 0.0f, -1.0f), deltaRoll); // roll 绕相机 Z 轴

    m_orientation = (yawQ * pitchQ * rollQ * m_orientation).normalized(); // 顺序合并旋转
}

void Camera::zoom(float offset)
{
    // offset>0 => fov 减小 => 视角变窄 => 视野放大
    setFOV(m_fov - offset);
}

// -------------------- 辅助：Euler <-> QQuaternion --------------------

QQuaternion Camera::eulerToQuat(float pitchDeg, float yawDeg, float rollDeg) const
{
    // QQuaternion 提供 fromEulerAngles(pitch, yaw, roll) (注意顺序 Y->Z->X)
    // 若要严格跟 glm 的 (yaw, pitch, roll) 对应，需要仔细确认
    // 这里先用 Qt 自带 fromEulerAngles, 它按照 (pitch, yaw, roll) 旋转顺序
    // Qt文档: "Rotation is performed around the X axis, then around the Y axis, and then around the Z axis."
    QQuaternion q = QQuaternion::fromEulerAngles(pitchDeg, yawDeg, rollDeg);
    return q.normalized();
}

QVector3D Camera::quatToEuler(const QQuaternion &quat) const
{
    // QQuaternion::toEulerAngles() 也有类似
    // 但 Qt 5.15+ 中才有 toEulerAngles()
    // 在较老版本 Qt 中可能需自己拆解
    // 这里直接:
    QVector3D euler = quat.toEulerAngles(); // pitch,yaw,roll in degrees
    return euler;
}
