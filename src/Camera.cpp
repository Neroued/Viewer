#include <Camera.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>

Camera::Camera()
    : m_position(0.0f, 0.0f, 3.0f),
      m_orientation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
      m_fov(45.0f),
      m_aspect(16.0f / 9.0f),
      m_nearPlane(0.1f),
      m_farPlane(100.0f),
      m_minFov(10.0f),
      m_maxFov(90.0f)
{
}

Camera::Camera(const glm::vec3 &position,
               const glm::quat &orientation,
               float fov,
               float aspect,
               float nearPlane,
               float farPlane)
    : m_position(position),
      m_orientation(orientation),
      m_fov(fov),
      m_aspect(aspect),
      m_nearPlane(nearPlane),
      m_farPlane(farPlane),
      m_minFov(1.0f),
      m_maxFov(90.0f)
{
    if (m_nearPlane <= 0.0f || m_farPlane <= m_nearPlane)
    {
        throw std::invalid_argument("Invalid clipping planes: nearPlane must be positive and less than farPlane.");
    }
}

void Camera::setPosition(const glm::vec3 &pos)
{
    m_position = pos;
}

void Camera::setOrientation(const glm::quat &orientation)
{
    m_orientation = glm::normalize(orientation);
}

void Camera::setOrientation(float pitch, float yaw, float roll)
{
    m_orientation = eulerToQuat(pitch, yaw, roll);
}

glm::vec3 Camera::getEulerOrientation() const
{
    return quatToEuler(m_orientation);
}

void Camera::setFOV(float fov)
{
    m_fov = glm::clamp(fov, m_minFov, m_maxFov);
}

void Camera::setAspect(float aspect)
{
    if (aspect <= 0.0f)
    {
        throw std::invalid_argument("Aspect ratio must be positive.");
    }
    m_aspect = aspect;
}

void Camera::setClippingPlanes(float nearPlane, float farPlane)
{
    if (nearPlane <= 0.0f || farPlane <= nearPlane)
    {
        throw std::invalid_argument("Invalid clipping planes: nearPlane must be positive and less than farPlane.");
    }
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

glm::mat4 Camera::getViewMatrix() const
{
    glm::mat4 rotationMatrix = glm::toMat4(glm::conjugate(m_orientation));
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -m_position);
    return rotationMatrix * translationMatrix;
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane);
}

void Camera::moveForward(float distance)
{
    glm::vec3 forward = glm::rotate(m_orientation, glm::vec3(0.0f, 0.0f, -1.0f));
    m_position += forward * distance;
}

void Camera::moveRight(float distance)
{
    glm::vec3 right = glm::rotate(m_orientation, glm::vec3(1.0f, 0.0f, 0.0f));
    m_position += right * distance;
}

void Camera::moveUp(float distance)
{
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    m_position += up * distance;
}

void Camera::rotate(const glm::vec3 &axis, float angleDeg)
{
    glm::quat rotation = glm::angleAxis(glm::radians(angleDeg), glm::normalize(axis));
    m_orientation = glm::normalize(rotation * m_orientation);
}

void Camera::rotate(float deltaPitch, float deltaYaw, float deltaRoll)
// 第一人称视角相机，俯仰绕自身X轴，其余绕世界轴
{
    glm::vec3 cameraRight = glm::rotate(m_orientation, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat pitchQuat = glm::angleAxis(glm::radians(deltaPitch), cameraRight);
    // glm::quat pitchQuat = glm::angleAxis(glm::radians(deltaPitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawQuat = glm::angleAxis(glm::radians(deltaYaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat rollQuat = glm::angleAxis(glm::radians(deltaRoll), glm::vec3(0.0f, 0.0f, 1.0f));
    m_orientation = glm::normalize(yawQuat * pitchQuat * rollQuat * m_orientation);
}

void Camera::zoom(float offset)
{
    m_fov = glm::clamp(m_fov - offset, m_minFov, m_maxFov);
}

glm::quat Camera::eulerToQuat(float pitchDeg, float yawDeg, float rollDeg) const
{
    glm::quat pitchQuat = glm::angleAxis(glm::radians(pitchDeg), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawQuat = glm::angleAxis(glm::radians(yawDeg), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat rollQuat = glm::angleAxis(glm::radians(rollDeg), glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::normalize(yawQuat * pitchQuat * rollQuat);
}

glm::vec3 Camera::quatToEuler(const glm::quat &q) const
{
    glm::vec3 euler = glm::eulerAngles(q);
    return glm::degrees(euler);
}

void Camera::normalizeOrientation()
{
    m_orientation = glm::normalize(m_orientation);
}