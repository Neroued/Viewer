cmake_minimum_required(VERSION 3.10)
project(Viewer)

set(CMAKE_CXX_STANDARD 17)

# 添加子目录和头文件路径
add_subdirectory(lib)
add_subdirectory(extern/glfw)
include_directories(extern/glfw/include)
include_directories(extern/glm)
include_directories(extern/glad/include)
include_directories(extern/imgui)
include_directories(include/)

# 添加 glad 和 imgui 静态库
file(GLOB GLAD_SOURCES extern/glad/src/*.c)
add_library(glad STATIC ${GLAD_SOURCES})
file(GLOB IMGUI_SOURCES extern/imgui/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})

# 创建静态库
file(GLOB LIB_SOURCES
    src/Application.cpp
)

add_library(ViewerLib STATIC ${LIB_SOURCES})

# 链接库所需的外部依赖
target_link_libraries(ViewerLib
    glfw
    glad
    imgui
    ${CMAKE_DL_LIBS}
    Lib
)

file(GLOB TEST_SOURCES
    tests/test.cpp
)

add_executable(test ${TEST_SOURCES})
target_link_libraries(test
    ViewerLib
    glfw
    glad
    imgui
    ${CMAKE_DL_LIBS}
    Lib
)
