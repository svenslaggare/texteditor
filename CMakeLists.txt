cmake_minimum_required(VERSION 3.10)
project(texteditor)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

set(SOURCE_FILES
    src/rendering/shadercompiler.cpp
    src/rendering/shadercompiler.h
    src/helpers.cpp
    src/helpers.h
    src/rendering/glhelpers.cpp
    src/rendering/glhelpers.h
    src/rendering/textrender.cpp
    src/rendering/textrender.h
    src/rendering/font.cpp
    src/rendering/font.h
    src/text/textformatter.cpp
    src/text/textformatter.h
    src/rendering/framebuffer.h
    src/rendering/framebuffer.cpp
    src/rendering/renderstyle.cpp
    src/rendering/renderstyle.h
    src/rendering/textview.cpp
    src/rendering/textview.h
    src/text/text.cpp
    src/text/text.h
    src/rendering/renderviewport.cpp
    src/rendering/renderviewport.h
    src/inputmanager.cpp
    src/inputmanager.h
    src/windowstate.cpp
    src/windowstate.h
    src/rendering/texturerender.cpp
    src/rendering/texturerender.h)

#add_compile_options(-O3 -march=native -mtune=native)

set(EXECUTABLE texteditor)
add_executable(${EXECUTABLE} ${SOURCE_FILES} src/main.cpp)

target_link_libraries(${EXECUTABLE} ${OPENGL_LIBRARY} ${GLFW3_LIBRARY} ${GLEW_LIBRARY} SOIL freetype)
target_include_directories(${EXECUTABLE} PRIVATE /usr/include/freetype2)
target_include_directories(${EXECUTABLE} PRIVATE /home/antjans/Dev/glm)