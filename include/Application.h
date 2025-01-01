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

    void addScene(std::shared_ptr<Scene> scene) { m_Scenes.push_back(scene); }
    void changeScene(int k);

    void onFramebufferSize(int width, int height);
    
    void run();
private:
    std::string m_title;
    float m_width, m_height;
    GLFWwindow *m_window;
    std::vector<std::shared_ptr<Scene>> m_Scenes;
    int currentScene;

    InputController m_inputController;

    void initializeGLFWAndGLAD();
    void bindInputController();
    void setInputControllerCamera();
};