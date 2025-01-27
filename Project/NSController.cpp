#include <NSController.h>

#include <Mesh.h>

#include <cmath>

using namespace FEMLib;

static double test_f(const VertexCoord *pos, double omega_0 = 1.0, double sigma = 1.0)
{
    using namespace std;

    double x = pos[0];
    double y = pos[1];
    double z = pos[2];

    double r_squared = x * x + y * y;
    // double theta = std::atan2(std::sqrt(x * x + y * y), z);
    double theta = std::atan2(y, x);
    double omega;

    // omega = 100 * z * std::exp(-50 * z * z) * (1 + 0.5 * cos(20 * theta));

    // 基于二维高斯分布生成涡量
    // omega = omega_0 * std::exp(-r_squared / (2.0 * sigma * sigma)) * (1.0 + 0.5 * std::cos(10.0 * theta) * z);

    // omega = 100 * z * std::exp(-50 * r_squared) * (1.0 + 0.5 * std::cos(20 * theta));
    omega = std::exp(-r_squared / (2.0 * sigma * sigma)) * std::sin(4 * theta);
    return omega;
}

NSController::NSController(int subdiv, MeshType type, QSharedPointer<Object> obj)
    : m_solver(subdiv, type), m_obj(obj), m_running(false), m_updated(false), m_dt(0.05), m_nu(std::pow(10, -2.0f))
{
    initData();
    m_obj->setObjectType(ObjectType::FEM);
    m_obj->setDrawMode(DrawMode::FILL);
    m_obj->loadFromMesh(m_solver.mesh);
    m_colorBufferFront = generateColors(m_solver.Omega);
    m_colorBufferBack = m_colorBufferFront;
    m_obj->setColorBuffer(m_colorBufferFront);
}

NSController::~NSController()
{
    stopComputeThread();
}

void NSController::initData()
{
    for (size_t i = 0; i < m_solver.Omega.size; ++i)
    {
        m_solver.Omega[i] = test_f(m_solver.mesh.vertex(i), 0.5, 1.5);
    }
}

void NSController::reset()
{
    if (m_running)
    {
        stopComputeThread();
        initData();
        m_colorBufferFront = generateColors(m_solver.Omega);
        m_obj->setColorBuffer(m_colorBufferFront);
        startComputeThread();
    }
    else
    {
        initData();
        m_colorBufferFront = generateColors(m_solver.Omega);
        m_obj->setColorBuffer(m_colorBufferFront);
    }
    m_updated = true;
}

void NSController::update(double dt)
{
    if (!m_updated)
        return;
    std::lock_guard<std::mutex> lock(m_dataMutex);
    if (m_updated)
    {
        // 用前缓冲区更新对象颜色数据
        m_obj->setColorBuffer(m_colorBufferFront);
        m_updated = false; // 重置更新标志
    }
}

void NSController::computeThread()
{
    while (m_running)
    {
        {
            std::unique_lock<std::mutex> lock(m_dataMutex);

            // 进行一次 Navier-Stokes 时间步推进
            m_solver.timeStep(m_nu, m_dt);

            // 根据新的涡量生成颜色缓冲区
            m_colorBufferBack = generateColors(m_solver.Omega);

            // 交换缓冲区（将后缓冲区数据拷贝到前缓冲区）
            std::swap(m_colorBufferFront, m_colorBufferBack);

            // 设置标记，表示颜色缓冲区已经更新
            m_updated = true;
        }

        // 为了避免占用过多 CPU，这里稍微 sleep 一下
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void NSController::startComputeThread()
{
    // 若已经在运行，则不重复启动
    if (m_running)
        return;

    m_running = true;
    m_computeThread = std::thread(&NSController::computeThread, this);
}

void NSController::stopComputeThread()
{
    // 将 running 标记为 false 并等待线程退出
    if (!m_running)
        return;
        
    m_running = false;

    if (m_computeThread.joinable())
    {
        m_computeThread.join();
    }
}

std::vector<float> NSController::generateColors(const Vec &u)
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