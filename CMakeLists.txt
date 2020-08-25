cmake_minimum_required(VERSION 3.6)
project(lameboi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(IMGUI_FOUND)
    include_directories("${IMGUI_INCLUDE_DIRS}")
else()
    add_subdirectory("${PROJECT_SOURCE_DIR}/lib/imgui")
    include_directories("${PROJECT_SOURCE_DIR}/lib/imgui")
    set(IMGUI_LIBRARIES "imgui")
endif()

if(FMT_FOUND)
    include_directories("${FMT_INCLUDE_DIRS}")
else()
    add_subdirectory("${PROJECT_SOURCE_DIR}/lib/fmt")
    include_directories("${PROJECT_SOURCE_DIR}/lib/fmt/fmt")
    set(FMT_LIBRARIES "fmt")
endif()

if(GL3W_FOUND)
    include_directories("${GL3W_INCLUDE_DIRS}")
else()
    add_subdirectory("${PROJECT_SOURCE_DIR}/lib/gl3w")
    include_directories("${PROJECT_SOURCE_DIR}/lib/gl3w")
    set(GL3W_LIBRARIES "gl3w")
endif()

include_directories("${PROJECT_SOURCE_DIR}/lib/PFD")
include_directories("${PROJECT_SOURCE_DIR}/lib/glfw/include")
include_directories("${PROJECT_SOURCE_DIR}/lib/SDL2/include")

# change these to use the architecture configuration
link_directories("${PROJECT_SOURCE_DIR}/lib/glfw/lib-vc2010-64")
link_directories("${PROJECT_SOURCE_DIR}/lib/SDL2/lib/x64")

file(GLOB_RECURSE SRC_LIST "src/*.cpp")
file(GLOB_RECURSE INCLUDE_LIST "src/*.h")
add_executable(lameboi ${SRC_LIST} ${INCLUDE_LIST})

target_include_directories(lameboi PUBLIC ./src)

target_link_libraries(lameboi 
    ${IMGUI_LIBRARIES} 
    ${FMT_LIBRARIES} 
    ${GL3W_LIBRARIES} 
    ${IMGUI_IMPL_LIBRARIES} 
    glfw3 
    opengl32
    SDL2 SDL2main
)