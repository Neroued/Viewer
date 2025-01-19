cmake_minimum_required(VERSION 3.16)
project(QtViewer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
qt_standard_project_setup()


# 源文件列表
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS include/*.h)
set(UI_FILE include/LeftPanel.ui)
set(SHADER_QRC resources/shaders/shaders.qrc)
set(ITEM_QRC resources/items/items.qrc)

qt_add_resources(RES_SOURCES ${SHADER_QRC} ${ITEM_QRC})


# 添加子目录和头文件路径
add_subdirectory(FEMLib)
include_directories(include/)
include_directories(extern/tinygltf)


# 需要头文件路径，用来添加Q_OBJECT宏
qt_add_executable(Qttest ${SOURCES} ${HEADERS} ${RES_SOURCES} ${UI_FILE} tests/Qttest.cpp)

# 关闭终端输出窗口
# set_target_properties(Qttest PROPERTIES WIN32_EXECUTABLE TRUE)

set(CMAKE_WIN32_EXECUTABLE ON)

# 链接 Qt6 库
target_link_libraries(Qttest PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    FEMLib
    OpenMP::OpenMP_CXX
    OpenGL::GL
)
