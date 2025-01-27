#include "LeftPanel.h"
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

#include <iostream>

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent), m_layout(nullptr), m_fpsLabel(nullptr), m_vertexCount(nullptr), m_faceCount(nullptr), m_startButton(nullptr), m_resetButton(nullptr), m_sceneButtonLayout(nullptr)
{
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

    // 切换场景的按钮
    // 使用一个vbox存放
    m_sceneButtonLayout = new QVBoxLayout();
    m_layout->addLayout(m_sceneButtonLayout);

    m_layout->addStretch(); // 添加弹性空间
}

// 槽函数，用于接收 FPS 数据
void LeftPanel::onFPSUpdated(float fps)
{
    // 更新 FPS 显示内容
    m_fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
}

// 设置顶点和面数信息
void LeftPanel::setInfo(const QVector<size_t> info)
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

void LeftPanel::onSceneListUpdated(const QStringList sceneList)
{
    // 更新场景对应的按钮
    // 清空旧的按钮
    QLayoutItem *child;
    while ((child = m_sceneButtonLayout->takeAt(0)) != nullptr)
    {
        if (child->widget())
        {
            delete child->widget();
        }
        delete child;
    }

    // 添加新的按钮
    for (const QString &sceneName : sceneList)
    {
        QPushButton *button = new QPushButton(sceneName, this);
        m_sceneButtonLayout->addWidget(button);

        // 使用信号槽将按钮与场景切换功能关联
        connect(button, &QPushButton::clicked, this, [this, sceneName]()
                {
                    emit changeScene(sceneName); // 发出信号，切换场景
                });
    }
}
