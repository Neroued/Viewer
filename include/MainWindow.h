#pragma once

#include <QMainWindow>

// 前置声明，减少编译时间，仅用于只需要指针的成员
class LeftPanel;
class QSplitter;
class OpenGLWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    OpenGLWidget *getOpenGLWidget() const { return m_openGLWidget; }

// private slots:
//     // 信号与槽，定义一些槽函数，用来与其他组件之间进行通信

private slots:

private:
    void initLayout();

    LeftPanel *m_leftPanel;       // 左侧的控制面板
    OpenGLWidget *m_openGLWidget; // 右侧OpenGL窗口
    QSplitter *m_splitter;        // 分割器
};
