#pragma once
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Font;
class RenderStyle;
class RenderViewPort;
class Text;

enum class FormatMode : std::uint8_t;

/**
 * Represents a text render
 */
class TextRender {
private:
	GLuint mVAO;
	GLuint mVBO;
	GLuint mShaderProgram;

	/**
	 * Sets the vertices for the given character
	 * @param charactersVertices The array of character vertices
	 * @param offset The offset for the character
	 * @param font The font
	 * @param character The character
	 * @param currentX The x position
	 * @param currentY The y position
	 * @param color The color
	 */
	void setCharacterVertices(GLfloat* charactersVertices,
							  std::size_t offset,
							  const Font& font,
							  char character,
							  float currentX,
							  float currentY,
							  glm::vec3 color);
public:
	/**
	 * Creates a new text render
	 * @param shaderProgram The shader program to render with
	 */
	explicit TextRender(GLuint shaderProgram);
	~TextRender();

	/**
	 * Renders the given text at the given position
	 * @param font The font
	 * @param formatMode The format mode
	 * @param renderStyle The render style
	 * @param viewPort The view port to render to
	 * @param text The text to render
	 * @param position The position to render at
	 */
	void render(const Font& font,
				FormatMode formatMode,
				const RenderStyle& renderStyle,
				const RenderViewPort& viewPort,
				const Text& text,
				glm::vec2 position);
};