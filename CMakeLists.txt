cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(lameboi)

set(LB_RESOURCE_DEST "${PROJECT_SOURCE_DIR}/bin" CACHE FILEPATH "Set the resource directory for debug builds.")

# change stack to be 10MB vs 1MB/8MB
if (MSVC)
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
else ()
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,10000000")
endif()

include_directories(
    "${PROJECT_SOURCE_DIR}/lib/PFD"
    "${PROJECT_SOURCE_DIR}/lib/imgui"
)

find_package(fmt CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
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
    fmt::fmt-header-only
    unofficial::gl3w::gl3w
    glfw
    imgui::imgui 
    nlohmann_json nlohmann_json::nlohmann_json
    opengl32
    SDL2::SDL2main SDL2::SDL2-static
)

# setup resource directory for debug purposes
file(MAKE_DIRECTORY ${LB_RESOURCE_DEST})

if(MSVC)
    # vs uses build root folder by default
    set_target_properties(lameboi PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${LB_RESOURCE_DEST})
endif()

source_group(TREE "${PROJECT_SOURCE_DIR}/src" PREFIX "Source Files/" FILES ${SRC_LIST})
source_group(TREE "${PROJECT_SOURCE_DIR}/src" PREFIX "Header Files/" FILES ${INCLUDE_LIST})