cmake_minimum_required(VERSION 3.10)
project(spooky)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Assuming your source files are in a 'src' directory. Adjust as needed.
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add the executable target
add_executable(spooky ${SOURCES})
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(include)
set(GLFW_ROOT_DIR /opt/homebrew/opt/glfw)
find_package(glfw3 REQUIRED PATHS ${GLFW_ROOT_DIR})
include_directories(${GLFW_ROOT_DIR}/include)
find_package(OpenGL REQUIRED)
set(GLEW_ROOT_DIR /opt/homebrew/Cellar/glew)
find_package(GLEW REQUIRED)
include_directories(${GLEW_ROOT_DIR}/include)
find_package(glm REQUIRED)
find_package(assimp CONFIG REQUIRED)
target_link_libraries(spooky glfw ${OPENGL_LIBRARIES} assimp::assimp GLEW::GLEW)
