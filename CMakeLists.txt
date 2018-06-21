cmake_minimum_required(VERSION 3.10)
project(texteditor)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

set(SOURCE_FILES
    src/shadercompiler.cpp
    src/shadercompiler.h
    src/helpers.cpp
    src/helpers.h
    src/glhelpers.cpp
    src/glhelpers.h
    src/textrender.cpp
    src/textrender.h
    src/font.cpp
    src/font.h
    src/textformatter.cpp
    src/textformatter.h
    src/framebuffer.h
    src/framebuffer.cpp
    src/renderstyle.cpp
    src/renderstyle.h
    src/textview.cpp
    src/textview.h
    src/text.cpp
    src/text.h
    src/renderviewport.cpp
    src/renderviewport.h
    src/inputmanager.cpp
    src/inputmanager.h
    src/windowstate.cpp
    src/windowstate.h
    src/texturerender.cpp
    src/texturerender.h)

#add_compile_options(-march=native -mtune=native)

set(EXECUTABLE texteditor)
add_executable(${EXECUTABLE} ${SOURCE_FILES} src/main.cpp)

target_link_libraries(${EXECUTABLE} ${OPENGL_LIBRARY} ${GLFW3_LIBRARY} ${GLEW_LIBRARY} SOIL freetype)
target_include_directories(${EXECUTABLE} PRIVATE /usr/include/freetype2)
target_include_directories(${EXECUTABLE} PRIVATE /home/antjans/Dev/glm)