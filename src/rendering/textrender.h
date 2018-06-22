#pragma once
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>

class Font;
class RenderStyle;
class RenderViewPort;
class FormattedText;

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

	/**
	 * Initializes the rendering
	 * @param font The font to render with
	 */
	void initializeRendering(const Font& font);

	void renderCurrentView(const Font& font,
						   std::size_t maxNumLines,
						   const RenderViewPort& viewPort,
						   glm::vec2 position,
						   std::function<void (std::int64_t, float&, float&)> renderLine);
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
	 * @param renderStyle The render style
	 * @param viewPort The view port to render to
	 * @param text The formatted text to render
	 * @param position The position to render at
	 */
	void render(const Font& font,
				const RenderStyle& renderStyle,
				const RenderViewPort& viewPort,
				const FormattedText& text,
				glm::vec2 position);

	/**
	 * Renders the line numbers
	 * @param font The font
	 * @param renderStyle The render style
	 * @param viewPort The view port to render to
	 * @param maxNumberLines The maximum number of lines
	 * @param position The position to render at
	 */
	void renderLineNumbers(const Font& font,
						   const RenderStyle& renderStyle,
						   const RenderViewPort& viewPort,
						   std::size_t maxNumberLines,
						   glm::vec2 position);
};