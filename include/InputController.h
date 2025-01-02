#pragma once
#define GLM_ENABLE_EXPERIMENTAL

/* 统一管理响应用户输入的控制器
 * 目前将用户的输入转换为相机视角的变化
 */

#include <Camera.h>
#include <unordered_map>

class InputController
{
public:
    InputController();
    InputController(Camera *camera);
    ~InputController() = default;

    void setCamera(Camera *camera);
    void setMoveSpeed(float speed);
    void setRotateSpeed(float speed);
    void setZoomSpeed(float speed);
    void setInvertY(bool invertY);

    void onKey(int key, int action, int mods);
    void onMouseButton(int button, int action, int mods);
    void onMouseMove(double xpos, double ypos);
    void onScroll(double xoffset, double yoffset);
    void update(float deltaTime);
    void reset();

private:
    Camera *m_camera;

    float m_moveSpeed;   // 移动速度
    float m_rotateSpeed; // 旋转灵敏度
    float m_zoomSpeed;   // 缩放灵敏度

    bool m_invertY; // 反转Y轴

    bool m_firstMouse; // 是否为第一次点击，避免突变
    double m_lastX;
    double m_lastY;

    bool m_leftButtonPressed;
    bool m_rightButtonPressed;

    std::unordered_map<int, bool> m_keyPressed; // 存储键盘按键状态

    void handleKeyboardMovement(float deltaTime);
    void pressKey(int key);
    void releaseKey(int key);
};