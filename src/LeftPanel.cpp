#include "LeftPanel.h"
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

#include <iostream>

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent), m_button(nullptr), m_label(nullptr), m_slider(nullptr), m_layout(nullptr), m_fpsLabel(nullptr)
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
    m_button = new QPushButton("按钮", this);
    m_label = new QLabel("标签内容", this);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_fpsLabel = new QLabel("FPS: 0", this); // 初始化 FPS 显示标签

    // 设置布局
    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_fpsLabel); // 添加 FPS 标签到布局
    m_layout->addWidget(m_button);
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_slider);
    m_layout->addStretch(); // 添加弹性空间
    setLayout(m_layout);
}

void LeftPanel::updateFPS(float fps)
{
    // 更新 FPS 显示内容
    m_fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
}

// 槽函数，用于接收 FPS 数据
void LeftPanel::onFPSUpdated(float fps)
{
    updateFPS(fps);
}