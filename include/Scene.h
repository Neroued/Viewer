#pragma once

/* 场景类
 * 拥有相机、对象、对象控制器等内容
 * 统一管理场景中的所有对象与对应的控制器
 * 从架构分层的角度来说，是Object的上一级，Application的下一级
 * 通过调用对象控制器的update方法更新对象的状态
 * 为Object对象的draw方法提供合适的opengl语境，并绘制这些对象
 * 可增加功能：根据相机视角剔除在视角外的对象
 */

#include <vector>
#include <string>
#include <memory>

#include <Object.h>
#include <Camera.h>
#include <ObjectController.h>
#include <Shader.h>

class Scene
{
private:
    std::string m_sceneName;
    Camera m_camera;
    std::vector<std::shared_ptr<Object>> m_Objects;
    std::vector<std::shared_ptr<ObjectController>> m_controllers;

    Shader m_backgroundShader;

public:
    Scene();
    Scene(const std::string &name);
    ~Scene();

    void setSceneName(const std::string &name) { m_sceneName = name; }
    std::string getSceneName() const { return m_sceneName; }

    void draw();
    void addObject(std::shared_ptr<Object> obj);

    void addController(std::shared_ptr<ObjectController> ctrl);
    void updateObjects(double dt);

    Camera &getCamera() { return m_camera; };

    void drawBackgroundAndGround(const glm::vec4& skyColor, const glm::vec3& groundColor);
};
