#include "LeftPanel.h"
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent), m_button(nullptr), m_label(nullptr), m_slider(nullptr), m_layout(nullptr)
{
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

    // 设置布局
    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_button);
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_slider);
    m_layout->addStretch(); // 添加弹性空间
    setLayout(m_layout);
}