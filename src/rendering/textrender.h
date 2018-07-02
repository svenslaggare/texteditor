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
class BaseFormattedText;
class InputState;

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
	void setupRendering(const Font& font);

	using RenderLine = std::function<void (GLfloat* vertices, std::size_t& offset, std::function<void()>, std::int64_t, glm::vec2&)>;

	/**
	 * Renders the current view
	 * @param font The font to use
	 * @param maxNumLines The maximum number of lines
	 * @param viewPort The view port
	 * @param position The position to render at
	 * @param renderLine Renders each line in the view
	 */
	void renderView(const Font& font,
					std::size_t maxNumLines,
					const RenderViewPort& viewPort,
					glm::vec2 position,
					RenderLine renderLine);

	/**
	 * Calculates the X position of the given index at the given line
	 * @param font The font
	 * @param renderStyle The render style
	 * @param text The text
	 * @param lineNumber The line number in the text
	 * @param offset The offset within the line
	 */
	float calculatePositionX(const Font& font,
							 const RenderStyle& renderStyle,
							 const BaseFormattedText& text,
							 std::size_t lineNumber,
							 std::size_t offset);
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
				const BaseFormattedText& text,
				glm::vec2 position);

	/**
	 * Renders the line numbers
	 * @param font The font
	 * @param renderStyle The render style
	 * @param viewPort The view port to render to
	 * @param text The formatted text to render lines for
	 * @param position The position to render at
	 */
	void renderLineNumbers(const Font& font,
						   const RenderStyle& renderStyle,
						   const RenderViewPort& viewPort,
						   const BaseFormattedText& text,
						   glm::vec2 position);

	/**
	 * Renders the caret
	 * @param font The font
	 * @param renderStyle The render style
	 * @param viewPort The view port to render to
	 * @param text The formatted text to caret for
	 * @param spacing The spacing
	 * @param inputState The input state
	 */
	void renderCaret(const Font& font,
					 const RenderStyle& renderStyle,
					 const RenderViewPort& viewPort,
					 const BaseFormattedText& text,
					 glm::vec2 spacing,
					 const InputState& inputState);
};