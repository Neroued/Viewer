#include "LeftPanel.h"
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

#include <iostream>

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent), m_startButton(nullptr), m_label(nullptr), m_slider(nullptr), m_layout(nullptr), m_fpsLabel(nullptr)
{
    // m_ui.setupUi(this);
    setupUI();
}

LeftPanel::~LeftPanel()
{
}

void LeftPanel::setupUI()
{
    // 初始化控件
    // 设置布局
    m_layout = new QVBoxLayout(this);

    // fps 与 顶点、面数
    m_fpsLabel = new QLabel("FPS: 0", this); // 初始化 FPS 显示标签
    m_vertexCount = new QLabel("Vertices: 0", this);
    m_faceCount = new QLabel("Faces: 0", this);

    m_layout->addWidget(m_fpsLabel); // 添加 FPS 标签到布局
    m_layout->addWidget(m_vertexCount);
    m_layout->addWidget(m_faceCount);

    // 控制计算的按钮
    m_startButton = new QPushButton("Start", this);
    m_layout->addWidget(m_startButton);
    connect(m_startButton, &QPushButton::clicked, this, &LeftPanel::onStartButtonCliked);

    // 重置状态的按钮
    m_resetButton = new QPushButton("Reset", this);
    m_layout->addWidget(m_resetButton);
    connect(m_resetButton, &QPushButton::clicked, this, &LeftPanel::onResetButtonCliked);

    m_layout->addStretch(); // 添加弹性空间
    setLayout(m_layout);
}

// 槽函数，用于接收 FPS 数据
void LeftPanel::onFPSUpdated(float fps)
{
    // 更新 FPS 显示内容
    m_fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
}

// 设置顶点和面数信息
void LeftPanel::setInfo(const QVector<size_t> &info)
{
    m_vertexCount->setText(QString("Vertices: %1").arg(info[0]));
    m_faceCount->setText(QString("Faces: %1").arg(info[1]));
}

void LeftPanel::onStartButtonCliked()
{
    if (started)
    {
        started = false;
        m_startButton->setText("Start");
        emit stopControllers();
    }
    else
    {
        started = true;
        m_startButton->setText("Stop");
        emit startControllers();
    }
}

void LeftPanel::onResetButtonCliked()
{
    emit resetControllers();
}
