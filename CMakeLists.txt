cmake_minimum_required(VERSION 3.20)

project(nanorender_opengl VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    microui
    GIT_REPOSITORY https://github.com/rxi/microui.git
    GIT_TAG 0850aba860959c3e75fb3e97120ca92957f9d057
)

FetchContent_MakeAvailable(glfw glm microui)

add_library(glad STATIC
    third_party/glad/src/gl.c
)
target_include_directories(glad PUBLIC third_party/glad/include)

add_library(microui_lib STATIC
    ${microui_SOURCE_DIR}/src/microui.c
)
target_include_directories(microui_lib PUBLIC ${microui_SOURCE_DIR}/src)

add_executable(nanorender_opengl
    src/gpu_ui_renderer.cpp
    src/gpu_ui_renderer.h
    src/mesh.cpp
    src/mesh.h
    src/mesh_renderer.cpp
    src/mesh_renderer.h
    src/main.cpp
    src/microui_atlas.c
    src/microui_atlas.h
    src/shader_program.cpp
    src/shader_program.h
    src/transform_controls.cpp
    src/transform_controls.h
)

target_include_directories(
    nanorender_opengl
    PRIVATE
        src
        ${microui_SOURCE_DIR}/demo
)

target_link_libraries(
    nanorender_opengl
    PRIVATE
        glad
        glfw
        glm::glm
        microui_lib
)

add_custom_command(
    TARGET nanorender_opengl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/models"
        "$<TARGET_FILE_DIR:nanorender_opengl>/models"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:nanorender_opengl>/shaders"
    COMMENT "Copying model and shader assets beside the executable"
)

if(MSVC)
    target_compile_options(nanorender_opengl PRIVATE /W4 /permissive-)
else()
    target_compile_options(nanorender_opengl PRIVATE -Wall -Wextra -Wpedantic)
endif()
