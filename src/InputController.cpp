#include <GLFW/glfw3.h>
#include <InputController.h>
#include <stdexcept>
#include <unordered_map>

// 常量定义
constexpr float DEFAULT_MOVE_SPEED = 1.5f;   // 默认移动速度
constexpr float DEFAULT_ROTATE_SPEED = 0.1f; // 默认旋转灵敏度
constexpr float DEFAULT_ZOOM_SPEED = 1.2f;   // 默认滚轮缩放速度
constexpr bool DEFAULT_INVERT_Y = false;     // 默认不反转鼠标 Y 轴

InputController::InputController()
    : m_camera(nullptr)
    , // 绑定要控制的相机
    m_moveSpeed(DEFAULT_MOVE_SPEED)
    , // 默认移动速度
    m_rotateSpeed(DEFAULT_ROTATE_SPEED)
    , // 默认鼠标旋转灵敏度
    m_zoomSpeed(DEFAULT_ZOOM_SPEED)
    , // 默认滚轮缩放速度
    m_invertY(DEFAULT_INVERT_Y)
    , // 默认不反转鼠标 Y 轴
    m_firstMouse(true)
    , // 初始鼠标状态
    m_lastX(0.0)
    , // 鼠标上一次 X 坐标
    m_lastY(0.0)
    , // 鼠标上一次 Y 坐标
    m_leftButtonPressed(false)
    ,                           // 默认左键未按下
    m_rightButtonPressed(false) // 默认右键未按下
{
    // 初始化按键状态
    m_keyPressed.clear();
}

InputController::InputController(Camera *camera)
    : m_camera(camera)
    , // 绑定要控制的相机
    m_moveSpeed(DEFAULT_MOVE_SPEED)
    , // 默认移动速度
    m_rotateSpeed(DEFAULT_ROTATE_SPEED)
    , // 默认鼠标旋转灵敏度
    m_zoomSpeed(DEFAULT_ZOOM_SPEED)
    , // 默认滚轮缩放速度
    m_invertY(DEFAULT_INVERT_Y)
    , // 默认不反转鼠标 Y 轴
    m_firstMouse(true)
    , // 初始鼠标状态
    m_lastX(0.0)
    , // 鼠标上一次 X 坐标
    m_lastY(0.0)
    , // 鼠标上一次 Y 坐标
    m_leftButtonPressed(false)
    ,                           // 默认左键未按下
    m_rightButtonPressed(false) // 默认右键未按下
{
    // 初始化按键状态
    m_keyPressed.clear();
}

void InputController::setMoveSpeed(float speed)
{
    if (speed <= 0.0f) {
        throw std::invalid_argument("Move speed must be positive.");
    }
    m_moveSpeed = speed;
}

void InputController::setRotateSpeed(float speed)
{
    if (speed <= 0.0f) {
        throw std::invalid_argument("Rotate speed must be positive.");
    }
    m_rotateSpeed = speed;
}

void InputController::setZoomSpeed(float speed)
{
    if (speed <= 0.0f) {
        throw std::invalid_argument("Zoom speed must be positive.");
    }
    m_zoomSpeed = speed;
}

void InputController::setInvertY(bool invertY)
{
    m_invertY = invertY;
}

void InputController::setCamera(Camera *camera)
{
    if (!camera) {
        throw std::invalid_argument("Camera pointer must not be null.");
    }
    m_camera = camera;
}

void InputController::onKey(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        pressKey(key);
    } else if (action == GLFW_RELEASE) {
        releaseKey(key);
    }
}

void InputController::pressKey(int key)
{
    m_keyPressed[key] = true;
}

void InputController::releaseKey(int key)
{
    m_keyPressed[key] = false;
}

void InputController::onMouseButton(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_leftButtonPressed = (action == GLFW_PRESS);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_rightButtonPressed = (action == GLFW_PRESS);
    }
}

void InputController::onMouseMove(double xpos, double ypos)
{
    if (!m_leftButtonPressed) {
        m_firstMouse = true;
        return; // 仅在左键按下时处理鼠标移动
    }
    if (m_firstMouse) {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }

    double xoffset = xpos - m_lastX;
    double yoffset = ypos - m_lastY;
    m_lastX = xpos;
    m_lastY = ypos;

    if (m_invertY) {
        yoffset = -yoffset;
    }

    if (m_camera) {
        m_camera->rotate(static_cast<float>(-yoffset * m_rotateSpeed),
                         static_cast<float>(-xoffset * m_rotateSpeed),
                         0.0f);
    }
}

void InputController::onScroll(double xoffset, double yoffset)
{
    if (m_camera) {
        m_camera->zoom(static_cast<float>(-yoffset * m_zoomSpeed));
    }
}

void InputController::update(float deltaTime)
{
    handleKeyboardMovement(deltaTime);
}

void InputController::handleKeyboardMovement(float deltaTime)
{
    if (!m_camera)
        return;

    glm::vec3 movement(0.0f);

    // 前后移动
    if (m_keyPressed[GLFW_KEY_W]) {
        movement.z += m_moveSpeed * deltaTime;
    }
    if (m_keyPressed[GLFW_KEY_S]) {
        movement.z -= m_moveSpeed * deltaTime;
    }

    // 左右移动
    if (m_keyPressed[GLFW_KEY_A]) {
        movement.x -= m_moveSpeed * deltaTime;
    }
    if (m_keyPressed[GLFW_KEY_D]) {
        movement.x += m_moveSpeed * deltaTime;
    }

    // 上下移动
    if (m_keyPressed[GLFW_KEY_Q]) {
        movement.y += m_moveSpeed * deltaTime;
    }
    if (m_keyPressed[GLFW_KEY_E]) {
        movement.y -= m_moveSpeed * deltaTime;
    }

    // 应用到相机
    if (glm::length(movement) > 0.0f) {
        m_camera->moveForward(movement.z);
        m_camera->moveRight(movement.x);
        m_camera->moveUp(movement.y);
    }
}

void InputController::reset()
{
    m_keyPressed.clear();
    m_firstMouse = true;
    m_leftButtonPressed = false;
    m_rightButtonPressed = false;
    m_lastX = 0.0;
    m_lastY = 0.0;
}
