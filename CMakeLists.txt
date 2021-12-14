cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(lameboi)

set(LB_RESOURCE_DEST "${PROJECT_SOURCE_DIR}/bin" CACHE FILEPATH "Set the resource directory for debug builds.")
set(LB_SHADER_DIR "${PROJECT_SOURCE_DIR}/src/frontend/shaders")

# change stack to be 10MB vs 1MB/8MB
if (MSVC)
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
else ()
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,10000000")
endif()

# todo: apply it for clang/gcc windows builds?
# NO MIN MAX MACROS >:(
if (MSVC)
    add_compile_definitions(NOMINMAX)
endif()

include_directories(
    "${PROJECT_SOURCE_DIR}/lib/PFD"
    "${PROJECT_SOURCE_DIR}/lib/imgui"
    "${PROJECT_SOURCE_DIR}/lib/stb"
)

find_package(fmt CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(lodepng CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

find_path(STB_INCLUDE_DIRS "stb.h")

file(GLOB_RECURSE SRC_LIST "src/*.cpp")
file(GLOB_RECURSE INCLUDE_LIST "src/*.h")
file(GLOB_RECURSE SHADER_LIST "${LB_SHADER_DIR}/*")

# todo: add "empty" str/file for UNIX builds
if (MSVC)
    file(GLOB_RECURSE RC_LIST "src/*.rc")
endif()

add_executable(lameboi 
    ${SRC_LIST} 
    ${INCLUDE_LIST}
    ${SHADER_LIST}
    ${RC_LIST}
)

set_target_properties(lameboi PROPERTIES
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(lameboi PRIVATE 
    ./src
    ${STB_INCLUDE_DIRS}
)

target_link_libraries(lameboi PRIVATE
    fmt::fmt-header-only
    unofficial::gl3w::gl3w
    glm::glm
    imgui::imgui 
    lodepng
    nlohmann_json nlohmann_json::nlohmann_json
    opengl32
    SDL2::SDL2main SDL2::SDL2-static
)

# setup resource directory for debug purposes
file(MAKE_DIRECTORY ${LB_RESOURCE_DEST})

# copy shader resources to our resource directory
foreach(file IN LISTS SHADER_LIST)
    string(REPLACE "${LB_SHADER_DIR}/" "" file ${file})
    configure_file("${LB_SHADER_DIR}/${file}" "${LB_RESOURCE_DEST}/shaders/${file}" COPYONLY)
endforeach()

# todo: add something for UNIX environments?
if(MSVC)
    # vs uses build root folder by default
    set_target_properties(lameboi PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${LB_RESOURCE_DEST})
endif()

source_group(TREE "${PROJECT_SOURCE_DIR}/src" FILES ${INCLUDE_LIST} ${SRC_LIST} ${RC_LIST})

#source_group(TREE "${PROJECT_SOURCE_DIR}/src" PREFIX "Source Files/" FILES ${SRC_LIST})
#source_group(TREE "${PROJECT_SOURCE_DIR}/src" PREFIX "Header Files/" FILES ${INCLUDE_LIST})
source_group(TREE "${LB_SHADER_DIR}" PREFIX "shaders" FILES ${SHADER_LIST})