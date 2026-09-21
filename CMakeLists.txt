# todo: I'd like to rewrite this to better support the core/render engine being detached from the frontend

cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(lameboi)

set(LB_RESOURCE_DEST "${PROJECT_SOURCE_DIR}/bin" CACHE FILEPATH "Set the resource directory for debug builds.")
set(LB_ASSET_DIR "${PROJECT_SOURCE_DIR}/src/assets")

# todo: apply it for clang/gcc windows builds?
# NO MIN MAX MACROS >:(
if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

find_package(fmt CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(lodepng CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(Stb REQUIRED)

file(GLOB_RECURSE SRC_LIST "src/*.cpp")
file(GLOB_RECURSE INCLUDE_LIST "src/*.h")
file(GLOB_RECURSE ASSET_LIST "${LB_ASSET_DIR}/*")

# todo: add "empty" str/file for UNIX builds
if (WIN32)
    file(GLOB_RECURSE RC_LIST "src/*.rc")
endif()

add_executable(lameboi 
    ${SRC_LIST} 
    ${INCLUDE_LIST}
    ${ASSET_LIST}
    ${RC_LIST}
)

set_target_properties(lameboi PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_compile_definitions(lameboi PRIVATE 
    IMGUI_USER_CONFIG="frontend/imconfig.h"
)

target_include_directories(lameboi PRIVATE 
    ./src
    ${Stb_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/lib/PFD
    ${PROJECT_SOURCE_DIR}/lib/imgui
)

target_link_libraries(lameboi PRIVATE
    fmt::fmt-header-only
    glad::glad
    glm::glm
    imgui::imgui 
	implot::implot
    lodepng
    nlohmann_json nlohmann_json::nlohmann_json
    opengl32
    SDL3::SDL3
)

# setup resource directory for debug purposes
file(MAKE_DIRECTORY ${LB_RESOURCE_DEST})

add_custom_command(
	TARGET lameboi POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${LB_ASSET_DIR} ${LB_RESOURCE_DEST}
)

# todo: add something for UNIX environments?
if(MSVC)
    # vs uses build root folder by default
    set_target_properties(lameboi PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${LB_RESOURCE_DEST})
endif()

source_group("src" FILES ${INCLUDE_LIST} ${SRC_LIST} ${RC_LIST})
source_group("assets" FILES ${ASSET_LIST})
