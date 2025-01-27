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

    // 初始化布局，左侧面板，右侧OpenGLWidget，中间使用Splitter分割
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

    connect(m_openGLWidget, &OpenGLWidget::fpsUpdated, m_leftPanel, &LeftPanel::onFPSUpdated);          // 更新fps
    connect(m_openGLWidget, &OpenGLWidget::vertexAndFaceInfoUpdated, m_leftPanel, &LeftPanel::setInfo); // 更新面数与顶点数

    connect(m_leftPanel, &LeftPanel::startControllers, m_openGLWidget, &OpenGLWidget::startCurrentSceneAllControllers);
    connect(m_leftPanel, &LeftPanel::stopControllers, m_openGLWidget, &OpenGLWidget::stopCurrentSceneAllControllers);
    connect(m_leftPanel, &LeftPanel::resetControllers, m_openGLWidget, &OpenGLWidget::resetCurrentSceneAllControllers);

    connect(m_openGLWidget, &OpenGLWidget::sceneListUpdated, m_leftPanel, &LeftPanel::onSceneListUpdated); // 更新场景列表对应的按钮
    connect(m_leftPanel, &LeftPanel::changeScene, m_openGLWidget, &OpenGLWidget::onChangeScene);
}
