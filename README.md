# Viewer

## 使用Qt框架重构

在原有基础上，添加一个MainWindow作为程序的主窗口，管理UI以及OpenGLWidget。

Application层改名为OpenGLWidget，继承自QOpenGLWidget，管理和提供opengl语境，并管理Scene对象。

Scene和Object需要增加initialize()方法，在被添加到OpenGLWidget后再初始化（上传VAO等数据）。

顺序：QApplication -> MainWindow -> 创建Object -> 创建场景 -> 将对象添加到场景中 -> 将场景添加到MainWindow -> 调用Scene的initialize()初始化场景和对象。







## 架构介绍

总体分为三层，Application - Scene - Object。

### Application类

Application类为向用户展示内容的主要对象，其中包含以下成员：

1. GLFW窗口指针，每一个Application类的实例分别管理一个窗口。
2. InputController类对象，负责处理用户输入，如鼠标键盘等操作，转化成相机的操作。
3. m_Scenes，存储需要展示的场景。

使用成员函数run()来渲染内容。

### Scene类

Scene类为存储场景信息的类。包含：

1. Camera类对象，为该场景的相机，在切换场景时可以保留之前的信息。
2. Objects，存储该场景中的所有Object。

Scene类提供函数在合适的由Application类提供的GLFW语境下渲染场景。

### Object类

Object类为存储需要渲染对象的类，是架构中的最底层单元。包含：

1. 渲染对象的网格信息。
2. 渲染对象的颜色信息（可选）。

可根据类型的设置选择不同的渲染方式，优化性能。
