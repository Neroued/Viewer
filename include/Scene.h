#pragma once

#include <vector>
#include <string>
#include <memory>

#include <Object.h>
#include <Camera.h>

class Scene
{
private:
    std::string m_sceneName;
    Camera m_camera;
    std::vector<std::shared_ptr<Object>> m_Objects;

public:
    Scene();
    Scene(const std::string &name);
    ~Scene();

    void setSceneName(const std::string &name) { m_sceneName = name; }
    std::string getSceneName() const { return m_sceneName; }

    void draw();
    void addObject(std::shared_ptr<Object> obj);

    Camera &getCamera() { return m_camera; };

    void drawBackgroundAndGround(const glm::vec4& skyColor, const glm::vec3& groundColor);
};
