#pragma once

#include <string>
#include <glm/vec2.hpp>

class Font;
class RenderViewPort;
class RenderStyle;
class TextRender;
class Text;
class InputState;

enum class FormatMode : std::uint8_t;

/**
 * Represents a text view
 */
class TextView {
private:
	const Font& mFont;
	FormatMode mFormatMode;
	const RenderViewPort& mViewPort;
	const RenderStyle& mRenderStyle;
public:
	/**
	 * Creates a new text view
	 * @param font The font
	 * @param formatMode The format mode
	 * @param viewPort The view port
	 * @param renderStyle The render style
	 */
	TextView(const Font& font, FormatMode formatMode, const RenderViewPort& viewPort, const RenderStyle& renderStyle);

	/**
	 * Renders the given text at the given position in the given view
	 * @param textRender The text text render
	 * @param text The text
	 * @param inputState The input state
	 * @param drawCaret Indicates if to draw the caret
	 */
	void render(TextRender& textRender, const Text& text, const InputState& inputState, bool drawCaret = true);
};