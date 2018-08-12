#pragma once
#include "shaderprogram.h"

#include <GL/glew.h>
#include <glm/vec2.hpp>

class WindowState;
class Font;
struct RenderStyle;
struct RenderViewPort;
class TextMetrics;
struct InputState;
class BaseFormattedText;

/**
 * Represents a render for text selections
 */
class TextSelectionRender {
private:
	ShaderProgram mShaderProgram;

	GLuint mVAO;
	GLuint mVBO;
public:
	TextSelectionRender();
	~TextSelectionRender();

	/**
	 * Renders the given text selection
	 * @param windowState The window state
	 * @param font The font
	 * @param viewPort The view port
	 * @param textMetrics The text metrics
	 * @param formattedText The formatted text
	 * @param offset The draw offset
	 * @param inputState The input state
	 */
	void render(const WindowState& windowState,
				const Font& font,
				const RenderViewPort& viewPort,
				const TextMetrics& textMetrics,
				const BaseFormattedText& formattedText,
				glm::vec2 offset,
				const InputState& inputState);
};