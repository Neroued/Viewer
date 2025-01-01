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

// void Application::run()
// {
//     Scene scene;
//     Camera &mainCamera = scene.getCamera();
//     m_inputController.setCamera(&mainCamera);
//     mainCamera.setAspect(m_width / m_height);

//     Mesh mesh(10, SPHERE);

//     auto shader = std::make_shared<Shader>();
//     shader->loadFromFile("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
//     shader->use();

//     auto obj = std::make_shared<Object>();
//     obj->loadFromMesh(mesh);
//     obj->setDrawMode(DrawMode::WIREFRAME);
//     obj->attachShader(shader);
//     auto model = obj->getModelMatrix();

//     auto obj2 = std::make_shared<Object>();
//     obj2->loadFromMesh(mesh);
//     obj2->setDrawMode(DrawMode::WIREFRAME);
//     obj2->setScale(glm::vec3(2.0f, 1.0f, 0.5f));
//     obj2->attachShader(shader);

//     scene.addObject(obj);
//     scene.addObject(obj2);

//     float lastTime = glfwGetTime();

//     // // 设置天空和地面的颜色
//     // const glm::vec4 skyColor(0.529f, 0.808f, 0.922f, 1.0f);    // 淡蓝色天空
//     // const glm::vec4 groundColor(0.933f, 0.867f, 0.510f, 1.0f); // 淡黄色地面

//     // GLuint groundVAO, groundVBO;
//     // GLfloat groundVertices[] = {
//     //     // 顶点位置             // 颜色
//     //     -100.0f, -2.0f, -100.0f, groundColor.r, groundColor.g, groundColor.b,
//     //     100.0f, -2.0f, -100.0f, groundColor.r, groundColor.g, groundColor.b,
//     //     100.0f, -2.0f, 100.0f, groundColor.r, groundColor.g, groundColor.b,
//     //     -100.0f, -2.0f, 100.0f, groundColor.r, groundColor.g, groundColor.b};
//     // GLuint groundIndices[] = {
//     //     0, 1, 2,
//     //     2, 3, 0};

//     // glGenVertexArrays(1, &groundVAO);
//     // glGenBuffers(1, &groundVBO);

//     // glBindVertexArray(groundVAO);

//     // glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
//     // glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

//     // GLuint groundEBO;
//     // glGenBuffers(1, &groundEBO);
//     // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
//     // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

//     // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)0);
//     // glEnableVertexAttribArray(0);
//     // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
//     // glEnableVertexAttribArray(1);

//     // glBindBuffer(GL_ARRAY_BUFFER, 0);
//     // glBindVertexArray(0);

//     while (!glfwWindowShouldClose(m_window))
//     {
//         glfwPollEvents();
//         float time = glfwGetTime();
//         float deltaTime = time - lastTime;
//         lastTime = time;
//         m_inputController.update(deltaTime);

//         // // 设置 OpenGL 的背景颜色为天空颜色
//         // glClearColor(skyColor.r, skyColor.g, skyColor.b, skyColor.a);
//         // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//         // auto view = mainCamera.getViewMatrix();
//         // auto projection = mainCamera.getProjectionMatrix();

//         // shader->setUniform("uView", view);
//         // shader->setUniform("uModel", model);
//         // shader->setUniform("uProjection", projection);

//         // // 绘制地面
//         // // glDisable(GL_DEPTH_TEST);
//         // shader->setUniform("drawmode", 1);
//         // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 设置地面为填充模式
//         // glBindVertexArray(groundVAO);
//         // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//         // glEnable(GL_DEPTH_TEST);

//         // // 使用 glm 的三角函数计算位置
//         // float x = 2.0f * glm::cos(time); // X 位置随着时间变化
//         // float y = 1.0f * glm::sin(time); // Y 位置随着时间变化
//         // float z = 0.0f;                  // Z 位置保持不变

//         scene.drawBackgroundAndGround(glm::vec4(0.529f, 0.808f, 0.922f, 1.0f), // 天空颜色（淡蓝色）
//                                       glm::vec3(0.752f, 0.752f, 0.752f));      // 地面颜色（淡灰色）)

//         scene.draw();

//         glfwSwapBuffers(m_window);
//     }

//     // glDeleteVertexArrays(1, &groundVAO);
//     // glDeleteBuffers(1, &groundVBO);
//     // glDeleteBuffers(1, &groundEBO);
// }

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
