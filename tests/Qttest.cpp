#include <QApplication>
#include "MainWindow.h"
#include <Object.h>
#include <Scene.h>
#include <iostream>
#include <NavierStokesSolver.h>
#include <QSharedPointer>
#include <ObjectController.h>
#include <OpenGLWidget.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

static double test_f(const Vec3 &pos, double omega_0 = 1.0, double sigma = 1.0)
{
    using namespace std;

    double x = pos[0];
    double y = pos[1];
    double z = pos[2];

    double r_squared = z * z;
    double theta = std::atan2(std::sqrt(x * x + y * y), z);
    double omega;

    // omega = 100 * z * std::exp(-50 * z * z) * (1 + 0.5 * cos(20 * theta));

    // 基于二维高斯分布生成涡量
    omega = omega_0 * std::exp(-r_squared / (2.0 * sigma * sigma)) * (1.0 + 0.5 * std::cos(10.0 * theta) * z);

    // omega = 100 * z * std::exp(-50 * r_squared) * (1.0 + 0.5 * std::cos(20 * theta));
    return omega;
}

class NSController : public ObjectController
{
public:
    NavierStokesSolver m_solver;
    QSharedPointer<Object> m_obj;
    bool updated;
    double nu;

    // 线程控制相关变量
    std::atomic<bool> running;             // 标志计算线程是否运行
    std::thread computeThread;             // 计算线程
    std::mutex dataMutex;                  // 数据访问互斥锁
    std::condition_variable dataCondition; // 数据同步条件变量

    std::vector<float> colorBufferFront; // 前缓冲区，用于渲染
    std::vector<float> colorBufferBack;  // 后缓冲区，用于计算

    NSController(int subdiv, MeshType type, QSharedPointer<Object> obj)
        : m_solver(subdiv, type), m_obj(obj), updated(true), nu(std::pow(10, -2.0f)), running(true)
    {

        for (size_t i = 0; i < m_solver.Omega.size; ++i)
        {
            m_solver.Omega[i] = test_f(m_solver.mesh.vertices[i], 0.5, 1.5);
        }

        m_obj->setObjectType(ObjectType::SEMI_STATIC);
        colorBufferFront = generateColors(m_solver.Omega);
        colorBufferBack = colorBufferFront; // 初始化后缓冲区
        m_obj->setColorBuffer(colorBufferFront);
        m_obj->loadFromMesh(m_solver.mesh);
        m_obj->setDrawMode(DrawMode::FILL);

        // 启动计算线程
        computeThread = std::thread(&NSController::computeLoop, this);
    }

    ~NSController()
    {
        // 停止计算线程
        running = false;
        if (computeThread.joinable())
        {
            computeThread.join();
        }
    }

    void update(double dt)
    {
        // 渲染线程中调用：将前缓冲区的数据加载到渲染对象
        std::unique_lock<std::mutex> lock(dataMutex);
        m_obj->setColorBuffer(colorBufferFront);
    }

private:
    void computeLoop()
    {
        while (running)
        {
            // 模拟计算步骤
            m_solver.timeStep(0.005, nu);

            // 生成新颜色数据并更新后缓冲区
            auto newColors = generateColors(m_solver.Omega);

            {
                // 加锁以同步缓冲区
                std::unique_lock<std::mutex> lock(dataMutex);
                colorBufferBack = newColors;
                std::swap(colorBufferFront, colorBufferBack); // 交换前后缓冲区
            }

            // 通知渲染线程更新数据
            dataCondition.notify_one();

            // 控制更新频率
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60FPS
        }
    }

    std::vector<float> generateColors(const Vec &u)
    {
        double u_min = *std::min_element(u.begin(), u.end());
        double u_max = *std::max_element(u.begin(), u.end());

        if (u_max == u_min)
        {
            // 全部设为蓝色
            std::vector<float> colors(u.size * 3);
            for (size_t i = 0; i < u.size; ++i)
            {
                colors[i * 3 + 0] = 0.0f; // R
                colors[i * 3 + 1] = 0.0f; // G
                colors[i * 3 + 2] = 1.0f; // B
            }
            return colors;
        }

        double mult = 1 / (u_max - u_min);

        std::vector<float> colors(u.size * 3); // 展平的颜色数组
        for (size_t i = 0; i < u.size; ++i)
        {
            // 归一化到 [0, 1]
            float normed = (u[i] - u_min) * mult;

            // 热力图映射（蓝绿红）
            if (normed < 0.5f)
            {
                colors[i * 3 + 0] = 0.0f;                 // R
                colors[i * 3 + 1] = normed * 2.0f;        // G
                colors[i * 3 + 2] = 1.0f - normed * 2.0f; // B
            }
            else
            {
                colors[i * 3 + 0] = (normed - 0.5f) * 2.0f;        // R
                colors[i * 3 + 1] = 1.0f - (normed - 0.5f) * 2.0f; // G
                colors[i * 3 + 2] = 0.0f;                          // B
            }
        }

        return colors;
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setVersion(4, 6);                        // 指定 OpenGL 版本，例如 3.3
    format.setProfile(QSurfaceFormat::CoreProfile); // 使用核心配置（或 setProfile(QSurfaceFormat::CompatibilityProfile)）
    format.setDepthBufferSize(24);                  // 设置深度缓冲大小
    format.setSamples(4);                           // 设置多重采样级别（4x MSAA）
    QSurfaceFormat::setDefaultFormat(format);       // 应用为默认格式

    MainWindow mainWindow;
    mainWindow.resize(800, 600); // 设置窗口大小
    mainWindow.show();           // 显示窗口
    std::cout << "mainWindow initialized" << std::endl;

    auto gl = mainWindow.getOpenGLWidget();

    auto scene = QSharedPointer<Scene>::create();
    std::cout << "created scene" << std::endl;
    scene->setShaderManager(gl->m_shaderManager);
    std::cout << "setShaderManager" << std::endl;

    Mesh mesh(10, SPHERE);

    // 创建两个网格对象
    auto obj = QSharedPointer<Object>::create();
    obj->setObjectType(ObjectType::STATIC);
    obj->loadFromMesh(mesh);
    obj->setDrawMode(DrawMode::WIREFRAME);
    obj->setShaderManager(gl->m_shaderManager);
    obj->setShader("basic");
    auto model = obj->getModelMatrix();

    Mesh mesh2(10, CUBE);
    auto obj2 = QSharedPointer<Object>::create();
    obj2->setObjectType(ObjectType::STATIC);
    obj2->loadFromMesh(mesh2);
    obj2->setDrawMode(DrawMode::WIREFRAME);
    // obj2->setScale(glm::vec3(2.0f, 1.0f, 0.5f));
    obj2->setPosition(QVector3D(2.0f, 0.0f, 2.0f));
    obj2->setShaderManager(gl->m_shaderManager);
    obj2->setShader("basic");

    scene->addObject(obj);
    scene->addObject(obj2);

    // 创建大量对象
    int n = 100;
    for (int i = 0; i < n; ++i)
    {
        auto obj = QSharedPointer<Object>::create();
        obj->setObjectType(ObjectType::STATIC);
        obj->loadFromMesh(mesh);
        obj->setDrawMode(DrawMode::WIREFRAME);
        obj->setShaderManager(gl->m_shaderManager);
        obj->setShader("basic");
        obj->setPosition(QVector3D(2.0f, (float)(i), 2.0f));
        scene->addObject(obj);
    }

    // 创建NS对象
    auto obj3 = QSharedPointer<Object>::create();
    obj3->setPosition(QVector3D(-2.0f, 0.0f, 2.0f));
    obj3->setShaderManager(gl->m_shaderManager);
    obj3->setShader("basic");
    QSharedPointer<NSController> nsController = QSharedPointer<NSController>::create(50, SPHERE, obj3);
    scene->addObject(obj3);
    scene->addController(nsController);

    // auto obj4 = std::make_shared<Object>();
    // std::shared_ptr<ObjectController> nsController2 = std::make_shared<NSController>(100, SPHERE, obj4);
    // obj4->setPosition(glm::vec3(-2.0f, 0.0f, 4.0f));
    // obj4->attachShader(shader);
    // scene->addObject(obj4);
    // scene->addController(nsController2);

    gl->addScene(scene);
    gl->initializeScenes();

    return app.exec(); // 启动应用程序事件循环
}
