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
    void onFPSUpdated(float fps);                         // 接收 FPS 信号的槽函数
    void setInfo(const QVector<size_t> info);             // 接收顶点和面数
    void onSceneListUpdated(const QStringList sceneList); // 接收sceneList创建对应的按钮

private slots:
    void onStartButtonCliked();
    void onResetButtonCliked();

signals:
    void startControllers();
    void stopControllers();
    void resetControllers();
    void changeScene(const QString name);

private:
    Ui::Form m_ui;
    void setupUI();

    QVBoxLayout *m_layout;

    QLabel *m_fpsLabel;    // 显示 FPS
    QLabel *m_vertexCount; // 显示顶点
    QLabel *m_faceCount;   // 显示面数

    QPushButton *m_startButton; // 控制计算是否开始的按钮
    bool started = false;

    QPushButton *m_resetButton;       // 重置状态的按钮
    QVBoxLayout *m_sceneButtonLayout; // 存放切换场景按钮的layout
};
