#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

/**
 * Represents a texture render
 */
class TextureRender {
private:
	GLuint mVAO;
	GLuint mVBO;
	GLuint mShaderProgram;
public:
	/**
	 * Creates a new texture render
	 * @param shaderProgram The shader program to render with
	 */
	explicit TextureRender(GLuint shaderProgram);
	~TextureRender();

	/**
	 * Renders the given texture
	 * @param texture The texture to draw
	 */
	void render(GLuint texture);
};