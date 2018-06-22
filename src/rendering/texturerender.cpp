#include <iostream>
#include "texturerender.h"
#include "glhelpers.h"

TextureRender::TextureRender(GLuint shaderProgram)
	: mShaderProgram(shaderProgram) {
	GLfloat vertices[] = {
		// Position    Texcoords
		-1.0f, 1.0f, 0.0f, 1.0f,  // Top-left
		1.0f, 1.0f, 1.0f, 1.0f,   // Top-right
		1.0f, -1.0f, 1.0f, 0.0f,  // Bottom-right
		-1.0f, -1.0f, 0.0f, 0.0f  // Bottom-left
	};

	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};

	GLHelpers::createVertexArrayObject(vertices, sizeof(vertices), elements, sizeof(elements), mVAO, mVBO);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glUseProgram(shaderProgram);

	auto posAttrib = glGetAttribLocation(shaderProgram, "vertexPosition");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

	auto texAttrib = glGetAttribLocation(shaderProgram, "vertexTexcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	glUniform1i(glGetUniformLocation(shaderProgram, "inputTexture"), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

TextureRender::~TextureRender() {
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

void TextureRender::render(GLuint texture) {
	glUseProgram(mShaderProgram);
	glBindVertexArray(mVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
