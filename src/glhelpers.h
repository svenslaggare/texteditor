#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <cstddef>
#include <memory>
#include <vector>

class Image;

struct GLUniform {
	std::size_t index;
	std::string name;
	GLint size;
	GLenum type;
};

std::ostream& operator<<(std::ostream& stream, const GLUniform& uniform);

/**
 * Contains helper functions for OpenGL
 */
namespace GLHelpers {
	/**
	 * Creates a new vertex array object
	 * @param vertices The vertices
	 * @param verticesSize The size of all the vertices
	 * @param elements The elements
	 * @param elementsSize The size of all the elements
	 * @param vao The vertex array object
	 * @param vbo The vertex buffer object
	 */
	void createVertexArrayObject(GLfloat* vertices,
								 std::size_t verticesSize,
								 GLuint* elements,
								 std::size_t elementsSize,
								 GLuint& vao,
								 GLuint& vbo);

	/**
	 * Returns the uniforms in the given shader program
	 * @param shaderProgram The shader program
	 */
	std::vector<GLUniform> getUniforms(GLuint shaderProgram);
}