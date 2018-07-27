#pragma once
#include <GL/glew.h>
#include <glm/vec2.hpp>

class WindowState;
class Font;
struct RenderStyle;
class TextMetrics;
struct InputState;
class BaseFormattedText;

/**
 * Represents a render for text selections
 */
class TextSelectionRender {
private:
	GLuint mVertexShader;
	GLuint mFragmentShader;
	GLuint mSelectionShaderProgram;

	GLuint mSelectionVAO;
	GLuint mSelectionVBO;
public:
	TextSelectionRender();
	~TextSelectionRender();

	/**
	 * Renders the given text selection
	 * @param windowState The window state
	 * @param font The font
	 * @param textMetrics The text metrics
	 * @param formattedText The formatted text
	 * @param offset The draw offset
	 * @param inputState The input state
	 */
	void render(const WindowState& windowState,
				const Font& font,
				const TextMetrics& textMetrics,
				const BaseFormattedText& formattedText,
				glm::vec2 offset,
				const InputState& inputState);
};