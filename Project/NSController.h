#pragma once

#include <Object.h>
#include <ObjectController.h>

#include <Mesh.h>
#include <NavierStokesSolver.h>

#include <QSharedPointer>

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>


class NSController : public ObjectController
{
public:
    FEMLib::NavierStokesSolver m_solver;
    QSharedPointer<Object> m_obj;
    bool m_updated;
    double m_dt, m_nu;

    std::vector<float> m_colorBufferFront; // 前缓冲区，用于渲染
    std::vector<float> m_colorBufferBack;  // 后缓冲区，用于计算

    // 线程控制相关
    std::atomic<bool> m_running;
    std::thread m_computeThread;
    std::mutex m_dataMutex;
    std::condition_variable m_dataCondition;

    NSController(int subdiv, FEMLib::MeshType type, QSharedPointer<Object> obj);
    ~NSController();

    void update(double dt);

    void start() { startComputeThread(); }
    void stop() { stopComputeThread(); }
    void reset();

private:
    std::vector<float> generateColors(const FEMLib::Vec &u);
    void initData();
    void computeThread();
    void startComputeThread();
    void stopComputeThread();
};