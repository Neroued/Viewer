#pragma once

#include <QWidget>

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

private:
    void setupUI();

    QPushButton *m_button; ///< 按钮
    QLabel *m_label;       ///< 标签
    QSlider *m_slider;     ///< 滑块
    QVBoxLayout *m_layout; ///< 布局
};
