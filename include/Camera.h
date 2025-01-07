#pragma once

#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

/**
 * @brief 一个使用 Qt 进行 3D 相机操作的类，替代原先的 glm 实现
 */
class Camera
{
public:
    Camera();
    Camera(const QVector3D &position,
           const QQuaternion &orientation,
           float fov,
           float aspect,
           float nearPlane,
           float farPlane);
    ~Camera() = default;

    // ------------------------
    // 位置和方向
    // ------------------------
    void setPosition(const QVector3D &pos);
    QVector3D getPosition() const { return m_position; }

    void setOrientation(const QQuaternion &orientation);
    QQuaternion getOrientation() const { return m_orientation; }

    // 使用欧拉角 (pitch, yaw, roll) 进行设置
    void setOrientation(float pitchDeg, float yawDeg, float rollDeg);
    QVector3D getEulerOrientation() const; // 以 (pitch,yaw,roll) 度数返回

    // ------------------------
    // 相机参数 (FoV, Aspect, Clip)
    // ------------------------
    void setFOV(float fovDeg);
    float getFOV() const { return m_fov; }

    void setAspect(float aspect);
    float getAspect() const { return m_aspect; }

    void setClippingPlanes(float nearPlane, float farPlane);
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }

    // ------------------------
    // 矩阵获取
    // ------------------------
    QMatrix4x4 getViewMatrix() const;
    QMatrix4x4 getProjectionMatrix() const;

    // ------------------------
    // 移动 & 旋转 & 缩放
    // ------------------------
    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);

    /**
     * @brief 围绕轴旋转
     * @param axis      旋转轴 (本例中将会做归一化)
     * @param angleDeg  旋转角度(度)
     */
    void rotate(const QVector3D &axis, float angleDeg);

    /**
     * @brief 分别绕本地 X, 全局Y, 全局Z 旋转 (或你可自定义第一人称规则)
     * @param deltaPitch   俯仰 (度)
     * @param deltaYaw     偏航 (度)
     * @param deltaRoll    翻滚 (度)
     */
    void rotate(float deltaPitch, float deltaYaw, float deltaRoll);

    /**
     * @brief 改变 FoV，用于相机缩放 (类似滚轮)
     * @param offset  正数=减小fov (放大)，负数=增大fov (缩小)
     */
    void zoom(float offset);

private:
    // ------------------------
    // 内部数据
    // ------------------------
    QVector3D m_position;      ///< 相机位置
    QQuaternion m_orientation; ///< 相机朝向 (四元数)

    float m_fov; ///< 垂直视场(度数)
    float m_aspect;
    float m_nearPlane;
    float m_farPlane;

    float m_minFov; ///< FoV下限
    float m_maxFov; ///< FoV上限

private:
    // ------------------------
    // 辅助函数
    // ------------------------
    QQuaternion eulerToQuat(float pitchDeg, float yawDeg, float rollDeg) const;
    QVector3D quatToEuler(const QQuaternion &quat) const;
};
