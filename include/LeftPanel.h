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

    void updateFPS(float fps);

public slots:
    void onFPSUpdated(float fps);      // 接收 FPS 信号的槽函数
    void setInfo(const QVector<size_t> &info); // 接收顶点和面数

private:
    Ui::Form m_ui;
    void setupUI();

    QPushButton *m_button; ///< 按钮
    QLabel *m_label;       ///< 标签
    QSlider *m_slider;     ///< 滑块
    QVBoxLayout *m_layout; ///< 布局
    QLabel *m_fpsLabel;    // 新增用于显示 FPS 的 QLabel
    QLabel *m_vertexCount;        // 显示顶点和面数
    QLabel *m_faceCount;
};
