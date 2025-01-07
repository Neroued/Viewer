#include "MainWindow.h"
#include "LeftPanel.h"
#include <QSplitter>
#include <OpenGLWidget.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_leftPanel(nullptr), m_openGLWidget(nullptr), m_splitter(nullptr)
{
    initLayout();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initLayout()
{
    m_leftPanel = new LeftPanel(this);
    m_openGLWidget = new OpenGLWidget(this);

    m_splitter = new QSplitter(this);

    m_splitter->addWidget(m_leftPanel);
    m_splitter->addWidget(m_openGLWidget);

    // 设置伸缩比例
    m_splitter->setStretchFactor(0, 1); // 左侧占较小比例
    m_splitter->setStretchFactor(1, 3); // 右侧占较大比例

    // 设置主窗口的中央部件
    setCentralWidget(m_splitter);
}