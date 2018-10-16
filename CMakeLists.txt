cmake_minimum_required(VERSION 3.10)
project(texteditor)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")

include(cmake/external.cmake)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

set(RENDERING_SOURCE_FILES
    src/rendering/common/framebuffer.cpp
    src/rendering/common/framebuffer.h
    src/rendering/common/glhelpers.cpp
    src/rendering/common/glhelpers.h
    src/rendering/common/shadercompiler.cpp
    src/rendering/common/shadercompiler.h
    src/rendering/common/shaderprogram.cpp
    src/rendering/common/shaderprogram.h
    src/rendering/font.cpp
    src/rendering/font.h
    src/rendering/renderstyle.cpp
    src/rendering/renderstyle.h
    src/rendering/renderviewport.cpp
    src/rendering/renderviewport.h
    src/rendering/textmetrics.cpp
    src/rendering/textmetrics.h
    src/rendering/textrender.cpp
    src/rendering/textrender.h
    src/rendering/textselectionrender.cpp
    src/rendering/textselectionrender.h
    src/rendering/texturerender.cpp
    src/rendering/texturerender.h)

set(TEXT_SOURCE_FILES
    src/text/formattedtext.cpp
    src/text/formattedtext.h
    src/text/formatters/cpp.cpp
    src/text/formatters/cpp.h
    src/text/formatters/python.cpp
    src/text/formatters/python.h
    src/text/formatters/text.cpp
    src/text/formatters/text.h
    src/text/helpers.cpp
    src/text/helpers.h
    src/text/incrementalformattedtext.cpp
    src/text/incrementalformattedtext.h
    src/text/text.cpp
    src/text/text.h
    src/text/textformatter.cpp
    src/text/textformatter.h)

set(INTERFACE_SOURCE_FILES
    src/interface/inputmanager.cpp
    src/interface/inputmanager.h
    src/interface/textview.cpp
    src/interface/textview.h)

set(EXTERNAL_SOURCE_FILES
    src/external/bloom_filter.hpp
    src/external/tsl/array_growth_policy.h
    src/external/tsl/array_hash.h
    src/external/tsl/array_map.h
    src/external/tsl/array_set.h)

set(MISC_SOURCE_FILES
    src/helpers.cpp
    src/helpers.h
    src/windowstate.cpp
    src/windowstate.h)

add_executable(
    texteditor
    ${RENDERING_SOURCE_FILES}
    ${TEXT_SOURCE_FILES}
    ${INTERFACE_SOURCE_FILES}
    ${MISC_SOURCE_FILES}
    ${EXTERNAL_SOURCE_FILES}
    src/main.cpp)
add_dependencies(texteditor glm)

target_link_libraries(texteditor ${OPENGL_LIBRARY} ${GLFW3_LIBRARY} ${GLEW_LIBRARY} freetype glfw)
target_include_directories(texteditor PRIVATE /usr/include/freetype2)
target_include_directories(texteditor PRIVATE ${GLM_INCLUDE_DIRS})