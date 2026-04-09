cmake_minimum_required(VERSION 3.10)
project(lvgl C)
set(CMAKE_C_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE LVGL_HEADERS
    "${PROJECT_SOURCE_DIR}/lv_drivers/*.h"
    "${PROJECT_SOURCE_DIR}/lv_examples/*.h"
    "${PROJECT_SOURCE_DIR}/lvgl/*.h"
    "${PROJECT_SOURCE_DIR}/*.h")

file(GLOB_RECURSE LVGL_SOURCES
    "${PROJECT_SOURCE_DIR}/lv_drivers/*.c"
    "${PROJECT_SOURCE_DIR}/lv_examples/*.c"
    "${PROJECT_SOURCE_DIR}/lvgl/src/*.c")

set(APP_SOURCES
    "${PROJECT_SOURCE_DIR}/main.c"
    "${PROJECT_SOURCE_DIR}/mouse_cursor_icon.c"
    "${PROJECT_SOURCE_DIR}/huge_list.c"
    "${PROJECT_SOURCE_DIR}/huge_list_demo.c")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

find_package(SDL2 REQUIRED SDL2)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(main ${APP_SOURCES} ${LVGL_SOURCES} ${LVGL_HEADERS})
target_link_libraries(main PRIVATE SDL2)
add_custom_target(run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main)
