#pragma once
#include "../helpers.h"
#include "inputmanager.h"
#include "../text/textformatter.h"
#include "../rendering/renderviewport.h"

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
 * Represents a keyboard command
 */
struct KeyboardCommand {
	int key;
	char normalMode; 		//When no modifier key is pressed
	char shiftMode;  		//When shift is being pressed
	char altMode;			//When alt is being pressed
};

/**
 * Defines how the formatting is performed
 */
enum class PerformFormattingType {
	Full,
	Partial
};

/**
 * Defines how character entries are handled
 */
enum class CharacterInputType {
	Native, // Uses the native GLFW methods
	Custom // Uses a custom implementation
};

/**
 * Represents a text view
 */
class TextView {
private:
	GLFWwindow* mWindow;

	PerformFormattingType mPerformFormattingType = PerformFormattingType::Partial;
	CharacterInputType mCharacterInputType = CharacterInputType::Native;

	const Font& mFont;
	FormatMode mFormatMode;

	const RenderViewPort& mViewPort;
	const RenderStyle& mRenderStyle;

	InputManager mInputManager;
	InputState mInputState;
	std::vector<KeyboardCommand> mKeyboardCommands;
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
	 * Returns the current line number
	 */
	std::size_t currentLineNumber() const;

	/**
	 * Returns the length of the current line
	 */
	std::size_t currentLineLength() const;

	/**
	 * Returns the number of lines
	 */
	std::size_t numLines();

	/**
	 * Returns the current line number and offset
	 * @param offsetX Additional offset
	 */
	std::pair<std::size_t, std::int64_t> getLineAndOffset(int offsetX = 0) const;

	/**
	 * Moves the caret in the x position by the given amount
	 * @param diff The amount to move
	 */
	void moveCaretX(std::int64_t diff);

	/**
	 * Moves the caret in the y position by the given amount
	 * @param diff The amount to move
	 */
	void moveCaretY(std::int64_t diff);

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

	/**
	 * Performs formatting on the view port
	 * @param viewPort The view port
	 * @param position The current position
	 */
	PartialFormattedText performPartialFormatting(const RenderViewPort& viewPort, glm::vec2 position);
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