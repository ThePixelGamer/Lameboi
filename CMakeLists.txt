cmake_minimum_required(VERSION 3.6)

set(CMAKE_TOOLCHAIN_FILE D:/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")
  
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(lameboi)

include_directories(
    "${PROJECT_SOURCE_DIR}/lib/PFD"
    "${PROJECT_SOURCE_DIR}/lib/imgui"
)

find_package(imgui CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

file(GLOB_RECURSE SRC_LIST "src/*.cpp")
file(GLOB_RECURSE INCLUDE_LIST "src/*.h")

add_executable(lameboi 
    ${SRC_LIST} 
    ${INCLUDE_LIST}
    "src/lameboi.rc"
)

target_include_directories(lameboi PRIVATE ./src)

target_link_libraries(lameboi PRIVATE
    imgui::imgui 
    fmt::fmt-header-only
    unofficial::gl3w::gl3w
    glfw
    opengl32
    SDL2::SDL2main SDL2::SDL2-static
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files//" FILES ${SRC_LIST})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Header Files//" FILES ${INCLUDE_LIST})