cmake_minimum_required(VERSION 3.27)
project(big2 C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_executable(big2_gui src/clients/gui.c src/clients/shared.c src/big2.c src/utils/pcg_basic.c src/utils/arena.c)
target_compile_options(big2_gui PUBLIC)
target_include_directories(big2_gui PUBLIC .)
target_link_libraries(big2_gui PUBLIC raylib)
if (APPLE)
  target_link_libraries(big2_gui PUBLIC "-framework IOKit")
  target_link_libraries(big2_gui PUBLIC "-framework Cocoa")
  target_link_libraries(big2_gui PUBLIC "-framework OpenGL")
endif()

add_executable(big2_cli src/clients/cli.c src/clients/shared.c src/big2.c src/utils/pcg_basic.c src/utils/arena.c)
target_compile_options(big2_cli PUBLIC)
target_include_directories(big2_cli PUBLIC .)

add_executable(big2_server src/server.c src/big2.c src/utils/pcg_basic.c src/utils/arena.c)
target_compile_options(big2_server PUBLIC)
target_include_directories(big2_server PUBLIC .)

if(MSVC)
  set(CMAKE_C_FLAGS_DEBUG "/D CLAY_DEBUG")
else()
  set(CMAKE_C_FLAGS_DEBUG "-ggdb -Wall -Wextra -Werror -pedantic -Wno-error=missing-braces -DCLAY_DEBUG")
  set(CMAKE_C_FLAGS_RELEASE "-O3")
endif()

add_custom_command(
        TARGET big2_gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources)
