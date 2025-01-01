#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

#include <Scene.h>
#include <InputController.h>

class Application
{
public:
    Application();
    ~Application();

    void run();
private:
    void initializeGLFWAndGLAD();

    GLFWwindow *m_window;
    std::string m_title;
    float m_width, m_height;
    std::vector<Scene> m_Scenes;

    InputController m_inputController;

    void bindInputController();
};