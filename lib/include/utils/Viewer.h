#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FEMdata.h>
#include <vector>
#include <algorithm>
#include <random>
#include <CSRMatrix.h>
#include <NavierStokesSolver.h>
#include <time.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

static void mouseCallback(GLFWwindow *window, double xpos, double ypos);
static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

class Viewer
{
public:
    Viewer(int width, int height, const char *title);
    ~Viewer();

    void initialize();                         // 初始化 OpenGL
    void runMesh();                            // 运行渲染循环
    void setupMesh(const Mesh &mesh);          // 显示 Mesh 网格
    void setupFEMData(const FEMData &femdata); // 显示FEMData
    void runFEMData();
    void setupNS(NavierStokesSolver &Solver);
    void runNS(double dt, double nu);
    void setupData(const Mesh &mesh, const Vec &data); // 在网格上显示data的数据
    void runData();

    GLFWwindow *window;
    unsigned int VAO, VBO, EBO, CBO; // VAO: 顶点数组对象; VBO: 顶点缓冲对象; CBO: 颜色缓冲对象; EBO: 索引缓冲对象
    unsigned int shaderProgram;
    int width, height;
    const Mesh *currentMesh;
    const FEMData *currentfemdata;
    NavierStokesSolver *currentNS;

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 4.5f);    // 相机位置
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // 目标位置
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);     // 上方向

    bool isMousePressed = false;              // 鼠标是否被按下
    double lastX = 0.0f, lastY = 0.0f;        // 上一次鼠标位置
    float yaw = -90.0f;                       // 偏航角（初始朝向 -Z 轴）
    float pitch = 0.0f;                       // 俯仰角
    float sensitivity = 0.15f;                // 鼠标灵敏度
    float rotationX = 0.0f, rotationY = 0.0f; // 累积旋转角度

    void initializeShaders();
    std::string loadShaderSource(const char *filepath);
    unsigned int compileShader(unsigned int type, const std::string &source); // 编译着色器
    std::vector<Vec3> generateColors(const Vec &u);                           // 生成颜色数据
    std::vector<Vec3> randomGenerateColors();
};

Viewer::Viewer(int width, int height, const char *title) : width(width), height(height), VAO(0), VBO(0), EBO(0), currentMesh(nullptr), currentfemdata(nullptr)
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8); // 设置抗锯齿多重采样级别

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    // 设置当前 Viewer 实例为窗口用户指针
    glfwSetWindowUserPointer(window, this);

    // 注册鼠标回调函数
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glEnable(GL_DEPTH_TEST);

    // 初始化着色器
    initializeShaders();
}

Viewer::~Viewer()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Viewer::initialize()
{
    // 设置清空颜色为白色背景
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void Viewer::initializeShaders()
{
    // 加载并编译顶点着色器
    std::string vertexCode = loadShaderSource("./shader/vertex_shader.glsl");
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode);

    // 加载并编译片段着色器
    std::string fragmentCode = loadShaderSource("./shader/fragment_shader.glsl");
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    // 链接着色器程序
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram); // 使用链接好的着色器程序
}

std::string Viewer::loadShaderSource(const char *filepath)
{
    std::ifstream file(filepath);
    if (!file)
    {
        throw std::runtime_error("Failed to open shader file");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Viewer::compileShader(unsigned int type, const std::string &source)
{
    unsigned int shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    return shader;
}

void Viewer::setupMesh(const Mesh &mesh)
{
    currentMesh = &mesh;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, currentMesh->vertex_count() * sizeof(Vec3), currentMesh->vertices.data, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, currentMesh->triangle_count() * 3 * sizeof(uint32_t), currentMesh->indices.data, GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(Vec3), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // 解绑 VAO
}

void Viewer::runMesh()
{
    if (!currentMesh)
        return;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        /// 通过旋转计算相机位置
        float radius = 4.5f; // 距离目标点的半径
        float camX = radius * cos(glm::radians(rotationX)) * cos(glm::radians(rotationY));
        float camY = -radius * sin(glm::radians(rotationY));
        float camZ = radius * sin(glm::radians(rotationX)) * cos(glm::radians(rotationY));

        cameraPos = glm::vec3(camX, camY, camZ);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        int isFEMDataLoc = glGetUniformLocation(shaderProgram, "isFEMData");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(isFEMDataLoc, GL_FALSE);

        // 绘制三角形面
        glUniform1i(glGetUniformLocation(shaderProgram, "isEdgeMode"), GL_FALSE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, currentMesh->triangle_count() * 3, GL_UNSIGNED_INT, 0);

        // 绘制边
        glUniform1i(glGetUniformLocation(shaderProgram, "isEdgeMode"), GL_TRUE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f); // 设置线宽
        glEnable(GL_LINE_SMOOTH);
        glDrawElements(GL_TRIANGLES, currentMesh->triangle_count() * 3, GL_UNSIGNED_INT, 0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Viewer::setupFEMData(const FEMData &femdata)
{
    currentfemdata = &femdata;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &CBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 设置顶点位置
    const Mesh &meshnow = currentfemdata->mesh;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, meshnow.vertex_count() * sizeof(Vec3), meshnow.vertices.data, GL_STATIC_DRAW);

    /* glVertexAttribPointer： 配置 VAO 中的位置数据的解析方式。
    参数：
    0： 顶点属性索引（Location 0，对应顶点着色器中的位置属性）。
    3： 每个顶点包含 3 个分量（x, y, z）。
    GL_DOUBLE： 数据类型为 double。
    GL_FALSE： 不需要归一化。
    sizeof(Vec3)： 每个顶点的步长（即每个顶点数据的总大小）。
    (void*)0： 顶点数据的起始偏移量，从缓冲区的起始位置开始。
    glEnableVertexAttribArray(0)： 启用顶点属性 Location 0。*/
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(Vec3), (void *)0);
    glEnableVertexAttribArray(0);

    // 生成颜色数据
    std::vector<Vec3> colors = generateColors(currentfemdata->u);
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(Vec3), colors.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(Vec3), (void *)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshnow.triangle_count() * 3 * sizeof(uint32_t),
                 meshnow.indices.data, GL_STATIC_DRAW);
}

std::vector<Vec3> Viewer::randomGenerateColors()
{
    // 初始化随机数生成器
    std::random_device rd;                                 // 随机种子
    std::mt19937 gen(rd());                                // 随机数生成器
    std::uniform_real_distribution<float> dis(0.0f, 1.0f); // 随机数范围 [0, 1]

    // 获取顶点数量
    const Vec &u = currentfemdata->u;
    size_t vertex_count = u.size;

    // 为每个顶点生成随机颜色
    std::vector<Vec3> colors(vertex_count);
    for (size_t i = 0; i < vertex_count; ++i)
    {
        colors[i] = Vec3(dis(gen), dis(gen), dis(gen)); // 随机生成 R, G, B 分量
    }

    return colors;
}

std::vector<Vec3> Viewer::generateColors(const Vec &u)
{
    double u_min = *std::min_element(u.begin(), u.end());
    double u_max = *std::max_element(u.begin(), u.end());
    if (u_max == u_min)
    {
        return std::vector<Vec3>(u.size, Vec3{0.0f, 0.0f, 1.0f}); // 全部设为蓝色
    }

    double mult = 1 / (u_max - u_min);

    std::vector<Vec3> colors(u.size);
    for (size_t i = 0; i < u.size; ++i)
    {
        // 归一化到[0,1]
        float normed = (u[i] - u_min) * mult;

        // 热力图映射（蓝绿红）
        if (normed < 0.5)
        {
            colors[i] = {0.0f, normed * 2.0f, 1.0f - normed * 2.0f};
        }
        else
        {
            colors[i] = {(normed - 0.5f) * 2.0f,
                         1.0f - (normed - 0.5f) * 2.0f,
                         0.0f};
        }
    }

    return colors;
}

void Viewer::runFEMData()
{
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // 通过旋转计算相机位置
        float radius = 4.5f; // 距离目标点的半径
        float camX = radius * cos(glm::radians(rotationX)) * cos(glm::radians(rotationY));
        float camY = -radius * sin(glm::radians(rotationY));
        float camZ = radius * sin(glm::radians(rotationX)) * cos(glm::radians(rotationY));

        cameraPos = glm::vec3(camX, camY, camZ);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

        // 获取 Uniform 变量位置
        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        int isEdgeModeLoc = glGetUniformLocation(shaderProgram, "isEdgeMode");
        int isFEMDataLoc = glGetUniformLocation(shaderProgram, "isFEMData");

        // 向着色器传递矩阵
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(isEdgeModeLoc, GL_FALSE);
        glUniform1i(isFEMDataLoc, GL_TRUE);

        // **绘制三角形面**
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 填充模式
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, currentfemdata->mesh.triangle_count() * 3, GL_UNSIGNED_INT, 0);

        // 解绑 VAO
        glBindVertexArray(0);

        // 交换缓冲区并处理事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Viewer::setupNS(NavierStokesSolver &Solver)
{
    currentNS = &Solver;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &CBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 设置顶点位置
    const Mesh &meshnow = currentNS->mesh;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, meshnow.vertex_count() * sizeof(Vec3), meshnow.vertices.data, GL_STATIC_DRAW);

    /* glVertexAttribPointer： 配置 VAO 中的位置数据的解析方式。
    参数：
    0： 顶点属性索引（Location 0，对应顶点着色器中的位置属性）。
    3： 每个顶点包含 3 个分量（x, y, z）。
    GL_DOUBLE： 数据类型为 double。
    GL_FALSE： 不需要归一化。
    sizeof(Vec3)： 每个顶点的步长（即每个顶点数据的总大小）。
    (void*)0： 顶点数据的起始偏移量，从缓冲区的起始位置开始。
    glEnableVertexAttribArray(0)： 启用顶点属性 Location 0。*/
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(Vec3), (void *)0);
    glEnableVertexAttribArray(0);

    // 生成颜色数据
    std::vector<Vec3> colors = generateColors(currentNS->Omega);
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(Vec3), colors.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(Vec3), (void *)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshnow.triangle_count() * 3 * sizeof(uint32_t),
                 meshnow.indices.data, GL_STATIC_DRAW);
}

void Viewer::runNS(double dt, double nu)
{
    // 颜色数据和线程同步相关变量
    std::vector<Vec3> colors = generateColors(currentNS->Omega); // 初始颜色
    std::mutex colorMutex;                                       // 保护颜色缓冲区
    std::atomic<bool> isRunning(true);                           // 标志程序是否运行
    std::atomic<bool> colorUpdated(false);                       // 标志颜色是否已更新

    // 启动计算线程
    std::thread computeThread([&]()
                              {
        while (isRunning) {
            // 计算时间步
            currentNS->timeStep(dt, nu);

            // 生成新的颜色数据
            std::vector<Vec3> newColors = generateColors(currentNS->Omega);

            // 更新颜色数据（加锁保护）
            {
                std::lock_guard<std::mutex> lock(colorMutex);
                colors = std::move(newColors);
                colorUpdated = true;
            }
        } });

    // 渲染线程
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // 获取当前时间，单位为秒
        float timeValue = glfwGetTime();
        float rotateSpeed = 0.5f;

        // 如果颜色已更新，更新颜色缓冲区
        if (colorUpdated)
        {
            std::lock_guard<std::mutex> lock(colorMutex);
            glBindBuffer(GL_ARRAY_BUFFER, CBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(Vec3), colors.data());
            colorUpdated = false;
        }

        // 通过旋转计算相机位置
        float radius = 4.0f; // 距离目标点的半径
        float camX = radius * cos(glm::radians(rotationX)) * cos(glm::radians(rotationY));
        float camY = -radius * sin(glm::radians(rotationY));
        float camZ = radius * sin(glm::radians(rotationX)) * cos(glm::radians(rotationY));

        cameraPos = glm::vec3(camX, camY, camZ);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        int isEdgeModeLoc = glGetUniformLocation(shaderProgram, "isEdgeMode");
        int isFEMDataLoc = glGetUniformLocation(shaderProgram, "isFEMData");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(isEdgeModeLoc, GL_FALSE);
        glUniform1i(isFEMDataLoc, GL_TRUE);

        // 绘制网格
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, currentNS->mesh.triangle_count() * 3, GL_UNSIGNED_INT, 0);

        // 交换缓冲区并处理事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 停止计算线程
    isRunning = false;
    computeThread.join();
}

// 鼠标移动回调函数
static void mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    Viewer *viewer = static_cast<Viewer *>(glfwGetWindowUserPointer(window));
    if (!viewer || !viewer->isMousePressed)
        return;

    float xoffset = xpos - viewer->lastX;
    float yoffset = viewer->lastY - ypos; // 鼠标 Y 方向反转
    viewer->lastX = xpos;
    viewer->lastY = ypos;

    // 更新旋转角度
    viewer->rotationX += xoffset * viewer->sensitivity;
    viewer->rotationY += yoffset * viewer->sensitivity;
}

// 鼠标按键回调函数
static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    Viewer *viewer = static_cast<Viewer *>(glfwGetWindowUserPointer(window));
    if (!viewer)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        viewer->isMousePressed = true;
        glfwGetCursorPos(window, &viewer->lastX, &viewer->lastY);
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        viewer->isMousePressed = false;
    }
}

void Viewer::setupData(const Mesh &mesh, const Vec &data) // 在网格上显示data的数据
{
    currentMesh = &mesh;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &CBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 设置顶点位置
    const Mesh &meshnow = mesh;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, meshnow.vertex_count() * sizeof(Vec3), meshnow.vertices.data, GL_STATIC_DRAW);

    /* glVertexAttribPointer： 配置 VAO 中的位置数据的解析方式。
    参数：
    0： 顶点属性索引（Location 0，对应顶点着色器中的位置属性）。
    3： 每个顶点包含 3 个分量（x, y, z）。
    GL_DOUBLE： 数据类型为 double。
    GL_FALSE： 不需要归一化。
    sizeof(Vec3)： 每个顶点的步长（即每个顶点数据的总大小）。
    (void*)0： 顶点数据的起始偏移量，从缓冲区的起始位置开始。
    glEnableVertexAttribArray(0)： 启用顶点属性 Location 0。*/
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(Vec3), (void *)0);
    glEnableVertexAttribArray(0);

    // 生成颜色数据
    std::vector<Vec3> colors = generateColors(data);
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(Vec3), colors.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(Vec3), (void *)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshnow.triangle_count() * 3 * sizeof(uint32_t),
                 meshnow.indices.data, GL_STATIC_DRAW);
}
void Viewer::runData()
{
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // 通过旋转计算相机位置
        float radius = 4.5f; // 距离目标点的半径
        float camX = radius * cos(glm::radians(rotationX)) * cos(glm::radians(rotationY));
        float camY = -radius * sin(glm::radians(rotationY));
        float camZ = radius * sin(glm::radians(rotationX)) * cos(glm::radians(rotationY));

        cameraPos = glm::vec3(camX, camY, camZ);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

        // 获取 Uniform 变量位置
        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        int isEdgeModeLoc = glGetUniformLocation(shaderProgram, "isEdgeMode");
        int isFEMDataLoc = glGetUniformLocation(shaderProgram, "isFEMData");

        // 向着色器传递矩阵
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(isEdgeModeLoc, GL_FALSE);
        glUniform1i(isFEMDataLoc, GL_TRUE);

        // **绘制三角形面**
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 填充模式
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, currentMesh->triangle_count() * 3, GL_UNSIGNED_INT, 0);

        // 解绑 VAO
        glBindVertexArray(0);

        // 交换缓冲区并处理事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}