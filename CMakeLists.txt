cmake_minimum_required(VERSION 3.10)
project(spooky)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(include)
include_directories(libs/imgui)
include_directories(libs/imgui/backends)

# Find packages
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
set(GLEW_ROOT_DIR /opt/homebrew/Cellar/glew)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(assimp CONFIG REQUIRED)

# ImGui sources
set(IMGUI_SOURCES
    libs/imgui/imgui.cpp libs/imgui/imgui_demo.cpp libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp libs/imgui/imgui_widgets.cpp)

# ImGui backend sources
set(IMGUI_BACKENDS libs/imgui/backends/imgui_impl_glfw.cpp
                   libs/imgui/backends/imgui_impl_opengl3.cpp)

# Your project's source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add executable
add_executable(spooky ${SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKENDS})

# Compiler options
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Link libraries
target_link_libraries(spooky glfw ${OPENGL_LIBRARIES} GLEW::GLEW assimp::assimp)
