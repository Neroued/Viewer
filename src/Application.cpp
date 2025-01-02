#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Scene.h>
#include <Application.h>
#include <Camera.h>
#include <InputController.h>
#include <Shader.h>

#include <Mesh.h>
#include <iostream>

Application::Application()
    : m_title("Default Title"), m_width(800), m_height(450), currentScene(0)
{
    initializeGLFWAndGLAD();
    bindInputController();
}

Application::~Application()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void Application::bindInputController()
{
    // 将 Application 的实例设置为窗口的用户指针
    glfwSetWindowUserPointer(m_window, this);

    // 设置键盘回调
    glfwSetKeyCallback(m_window, [](GLFWwindow *win, int key, int scancode, int action, int mods)
                       {
        auto app = static_cast<Application*>(glfwGetWindowUserPointer(win));
        if (app) {
            app->m_inputController.onKey(key, action, mods);
        } });

    // 设置鼠标按键回调
    glfwSetMouseButtonCallback(m_window, [](GLFWwindow *win, int button, int action, int mods)
                               {
        auto app = static_cast<Application*>(glfwGetWindowUserPointer(win));
        if (app) {
            app->m_inputController.onMouseButton(button, action, mods);
        } });

    // 设置鼠标移动回调
    glfwSetCursorPosCallback(m_window, [](GLFWwindow *win, double xpos, double ypos)
                             {
        auto app = static_cast<Application*>(glfwGetWindowUserPointer(win));
        if (app) {
            app->m_inputController.onMouseMove(xpos, ypos);
        } });

    // 设置滚轮回调
    glfwSetScrollCallback(m_window, [](GLFWwindow *win, double xoffset, double yoffset)
                          {
        auto app = static_cast<Application*>(glfwGetWindowUserPointer(win));
        if (app) {
            app->m_inputController.onScroll(xoffset, yoffset);
        } });

    // 设置窗口大小回调
    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow *win, int width, int height)
                                   {
        auto app = static_cast<Application*>(glfwGetWindowUserPointer(win));
        if (app) {
            app->onFramebufferSize(width, height);
        } });
}

void Application::onFramebufferSize(int width, int height)
    {
        // 调整视口大小
        glViewport(0, 0, width, height);
        m_height = height;
        m_width = width;
        m_Scenes[currentScene]->getCamera().setAspect(m_width / m_height);
    }


void Application::setInputControllerCamera()
{
    if (m_Scenes.size() == 0)
    {
        std::cerr << "Error: No Scene Exists." << std::endl;
        return;
    }

    if (currentScene >= m_Scenes.size())
    {
        std::cerr << "Error: Invalid Scene Index." << std::endl;
        currentScene = 0;
    }

    Camera &cam = m_Scenes[currentScene]->getCamera();
    m_inputController.setCamera(&cam);
    m_inputController.reset();
    cam.setAspect(m_width / m_height);
}

void Application::run()
{
    setInputControllerCamera();
    float lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        // 计算dt
        float time = glfwGetTime();
        float dt = time - lastTime;
        lastTime = time;
        m_inputController.update(dt);
        m_Scenes[currentScene]->updateObjects(dt);
        m_Scenes[currentScene]->drawBackgroundAndGround(glm::vec4(0.529f, 0.808f, 0.922f, 1.0f), // 天空颜色（淡蓝色）
                                                        glm::vec3(0.752f, 0.752f, 0.752f));      // 地面颜色（淡灰色）)
        m_Scenes[currentScene]->draw();

        glfwSwapBuffers(m_window);
    }
}

void Application::initializeGLFWAndGLAD()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // 设置 OpenGL 上下文版本和窗口属性
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8); // 设置抗锯齿多重采样级别

    // 创建窗口
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
    if (!m_window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
}
