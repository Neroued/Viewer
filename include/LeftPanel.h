#pragma once

#include <QWidget>
#include <QVector>
#include "ui_LeftPanel.h"

class QPushButton;
class QLabel;
class QSlider;
class QVBoxLayout;

class LeftPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LeftPanel(QWidget *parent = nullptr);
    ~LeftPanel();


public slots:
    void onFPSUpdated(float fps);              // 接收 FPS 信号的槽函数
    void setInfo(const QVector<size_t> &info); // 接收顶点和面数

private slots:
    void onStartButtonCliked();
    void onResetButtonCliked();

signals:
    void startControllers();
    void stopControllers();
    void resetControllers();


private:
    Ui::Form m_ui;
    void setupUI();

    QLabel *m_fpsLabel;         // 显示 FPS
    QLabel *m_vertexCount;      // 显示顶点
    QLabel *m_faceCount;        // 显示面数

    QPushButton *m_startButton; // 控制计算是否开始的按钮
    bool started = false;

    QPushButton *m_resetButton; // 重置状态的按钮

    QLabel *m_label;            ///< 标签
    QSlider *m_slider;          ///< 滑块
    QVBoxLayout *m_layout;      ///< 布局
};
