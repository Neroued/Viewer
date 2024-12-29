cmake_minimum_required(VERSION 3.10)
project(Viewer)

set(CMAKE_CXX_STANDARD 17)

# 添加子目录和头文件路径
add_subdirectory(extern/glfw)
include_directories(extern/glfw/include)
include_directories(extern/glm)
include_directories(extern/glad/include)
include_directories(extern/imgui)

# 添加 glad 和 imgui 静态库
file(GLOB GLAD_SOURCES extern/glad/src/*.c)
add_library(glad STATIC ${GLAD_SOURCES})
file(GLOB IMGUI_SOURCES extern/imgui/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})

# 主程序源文件
file(GLOB SOURCES src/*.cpp)

# 创建可执行文件并链接库
add_executable(Viewer ${SOURCES})
target_link_libraries(Viewer glfw glad imgui)
