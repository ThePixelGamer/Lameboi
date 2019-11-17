file(GLOB SRC_LIST "*.cpp")
file(GLOB INCLUDE_LIST "*.h")

add_library(imgui STATIC ${SRC_LIST} ${INCLUDE_LIST} examples/imgui_impl_opengl3.cpp examples/imgui_impl_glfw.cpp)

target_include_directories(imgui PUBLIC ./)
target_include_directories(imgui PUBLIC ./examples)
target_include_directories(imgui PUBLIC ../glfw/include)
target_include_directories(imgui PUBLIC ../gl3w)