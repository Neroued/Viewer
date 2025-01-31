cmake_minimum_required(VERSION 3.16)
project(QtViewer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_VSCODE_OUTPUT "Use ./build/vscode as output directory" OFF)

if(ENABLE_VSCODE_OUTPUT)
    message(STATUS "Enabling custom output directories for VSCode build.")

    # 对单配置生成器来说, 这里的 CMAKE_BUILD_TYPE 通常是 Debug 或 Release
    set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/vscode/${CMAKE_BUILD_TYPE})

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
endif()

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install path" FORCE)


# ============ find package, add_subdirectory 等内容 ============
find_package(OpenMP REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
qt_standard_project_setup()


# 源文件列表
file(GLOB SOURCES src/*.cpp Project/MatrixVisualizer.cpp Project/NSController.cpp)
file(GLOB HEADERS include/*.h Project/MatrixVisualizer.h Project/NSController.h)
set(UI_FILE include/LeftPanel.ui)
set(SHADER_QRC resources/shaders/shaders.qrc)
set(ITEM_QRC resources/items/items.qrc)

qt_add_resources(RES_SOURCES ${SHADER_QRC} ${ITEM_QRC})


# 添加子目录和头文件路径
add_subdirectory(FEMLib)
include_directories(include/)
include_directories(extern/tinygltf)
include_directories(Project/)


# 需要头文件路径，用来添加Q_OBJECT宏
qt_add_executable(Qttest ${SOURCES} ${HEADERS} ${RES_SOURCES} ${UI_FILE} tests/Qttest.cpp)
qt_add_executable(MV ${SOURCES} ${HEADERS} ${RES_SOURCES} ${UI_FILE} tests/MV.cpp)
qt_add_executable(Project ${SOURCES} ${HEADERS} ${RES_SOURCES} ${UI_FILE} Project/project.cpp)


# 关闭终端输出窗口
# set_target_properties(Qttest PROPERTIES WIN32_EXECUTABLE TRUE)

# set(CMAKE_WIN32_EXECUTABLE ON)

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

target_link_libraries(MV PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    FEMLib
    OpenMP::OpenMP_CXX
    OpenGL::GL
)

target_link_libraries(Project PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    FEMLib
    OpenMP::OpenMP_CXX
    OpenGL::GL
)

# ============ windeployqt 部署命令 =============
set(QT_DEPLOY_TOOL E:/Qt/6.8.1/mingw_64/bin/windeployqt6.exe)

add_custom_command(
    TARGET Qttest POST_BUILD
    COMMAND ${QT_DEPLOY_TOOL} --dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} $<TARGET_FILE:Qttest>
    COMMENT "Running windeployqt6.exe for Qttest"
)

add_custom_command(
    TARGET MV POST_BUILD
    COMMAND ${QT_DEPLOY_TOOL} --dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} $<TARGET_FILE:MV>
    COMMENT "Running windeployqt6.exe for MV"
)

add_custom_command(
    TARGET Project POST_BUILD
    COMMAND ${QT_DEPLOY_TOOL} --dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} $<TARGET_FILE:Project>
    COMMENT "Running windeployqt6.exe for Project"
)



# ============ install 部分 ============
install(TARGETS Qttest MV Project
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 注意这里的 install(DIRECTORY) 会把 output 目录下所有文件都拷贝到 bin
# 一般足够简单的场景可以接受
install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    DESTINATION bin
    FILES_MATCHING PATTERN "*.*"
)