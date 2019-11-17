file(GLOB_RECURSE SRC_LIST "GL/*.c")

add_library(gl3w STATIC ${SRC_LIST})

target_include_directories(gl3w PUBLIC ./)