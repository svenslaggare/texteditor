#include "glhelpers.h"

#include <iostream>
#include <cstring>

std::ostream& operator<<(std::ostream& stream, const GLUniform& uniform) {
	stream << "Uniform " << uniform.index << ": type: " << uniform.type << ", name: " << uniform.name;
	return stream;
}

void GLHelpers::createVertexArrayObject(GLfloat* vertices,
										std::size_t verticesSize,
										GLuint* elements,
										std::size_t elementsSize,
										GLuint& vao,
										GLuint& vbo) {
	// Create Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vertex Buffer Object and copy the vertex data to it
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

	// Create an element array
	GLuint ebo;
	glGenBuffers(1, &ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementsSize, elements, GL_STATIC_DRAW);
}

std::vector<GLUniform> GLHelpers::getUniforms(GLuint shaderProgram) {
	std::vector<GLUniform> uniforms;

	int count;
	glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &count);

	for (int i = 0; i < count; i++)	{
		GLint size; // size of the variable
		GLenum type; // type of the variable (float, vec3 or mat4, etc)

		const GLsizei bufferSize = 32;
		GLchar name[bufferSize];
		GLsizei length;
		glGetActiveUniform(shaderProgram, (GLuint)i, bufferSize, &length, &size, &type, name);

		uniforms.emplace_back();
		auto& uniform = uniforms.back();

		uniform.index = (std::size_t)i;
		uniform.name = name;
		uniform.size = size;
		uniform.type = type;
	}

	return uniforms;
}
