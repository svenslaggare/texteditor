#pragma once
#include "../helpers.h"
#include "../inputmanager.h"
#include "../text/textformatter.h"
#include "renderviewport.h"

#include <string>
#include <glm/vec2.hpp>

class Font;
class RenderStyle;
class TextRender;
class Text;
class InputState;

enum class FormatMode : std::uint8_t;

/**
 * The input state
 */
struct InputState {
	glm::vec2 viewPosition = glm::vec2();
	std::int64_t caretPositionX = 0;
	std::int64_t caretPositionY = 0;
};

/**
 * Represents a text view
 */
class TextView {
private:
	GLFWwindow* mWindow;

	const Font& mFont;
	FormatMode mFormatMode;

	const RenderViewPort& mViewPort;
	const RenderStyle& mRenderStyle;

	InputManager mInputManager;
	InputState mInputState;
	const float mScrollSpeed = 4.0f;

	FormattedText mFormattedText;
	RenderViewPort mLastViewPort;
	std::size_t mTextVersion = 0;
	Text& mText;

	bool mDrawCaret = false;
	TimePoint mLastCaretUpdate;

	/**
	 * Returns the current line the caret is at
	 */
	const LineTokens& currentLine() const;

	/**
	 * Moves the caret in the y position by the given amount
	 * @param diff The amount to move
	 */
	void moveCaretY(int diff);

	/**
	 * Updates the input
	 * @param windowState The window state
	 */
	void updateInput(const WindowState& windowState);

	/**
	 * Updates the editing
	 * @param windowState The window state
	 */
	void updateEditing(const WindowState& windowState);

	/**
	 * Updates the view movement
	 * @param windowState The window state
	 */
	void updateViewMovement(const WindowState& windowState);

	/**
	 * Returns the spacing due to line numbers
	 */
	float getLineNumberSpacing() const;

	/**
	 * Returns the view port for the text part
	 */
	RenderViewPort getTextViewPort();

	/**
	 * Updates the formatted text
	 * @param viewPort The view port
	 */
	void updateFormattedText(const RenderViewPort& viewPort);
public:
	/**
	 * Creates a new text view
	 * @param window The window
	 * @param font The font
	 * @param formatMode The format mode
	 * @param viewPort The view port
	 * @param renderStyle The render style
	 * @param text The text
	 */
	TextView(GLFWwindow* window,
			 const Font& font,
			 FormatMode formatMode,
			 const RenderViewPort& viewPort,
			 const RenderStyle& renderStyle,
			 Text& text);

	/**
	 * Updates the text view
	 * @param windowState The window state
	 */
	void update(const WindowState& windowState);

	/**
	 * Renders the current view
	 * @param textRender The text text render
	 */
	void render(TextRender& textRender);
};