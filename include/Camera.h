#pragma once
#define GLM_ENABLE_EXPERIMENTAL

/* 第一人称自由相机
 * 使用四元数处理旋转
 */

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

class Camera
{
public:
    Camera();
    Camera(const glm::vec3 &position,
           const glm::quat &orientation,
           float fov,
           float aspect,
           float nearPlane,
           float farPlane);
    ~Camera() = default;

    void setPosition(const glm::vec3 &pos);
    glm::vec3 getPosition() const { return m_position; }

    void setOrientation(const glm::quat &orientation);
    glm::quat getOrientation() const { return m_orientation; }

    // 使用欧拉角
    void setOrientation(float pitch, float yaw, float roll);
    glm::vec3 getEulerOrientation() const;

    void setFOV(float FOV);
    float getFOV() const { return m_fov; }

    void setAspect(float aspect);
    float getAspect() const { return m_aspect; }
    
    void setClippingPlanes(float nearPlane, float farPlane);
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    void rotate(const glm::vec3 &axis, float angleDeg);
    void rotate(float deltaPitch, float deltaYaw, float deltaRoll);
    void zoom(float offset);

private:
    glm::vec3 m_position;
    glm::quat m_orientation;

    float m_fov;
    float m_aspect;
    float m_nearPlane;
    float m_farPlane;

    float m_minFov;
    float m_maxFov;

    glm::quat eulerToQuat(float pitch, float yaw, float roll) const;
    glm::vec3 quatToEuler(const glm::quat &q) const;
    void normalizeOrientation();
};