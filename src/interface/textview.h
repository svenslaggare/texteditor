#pragma once
#include "../helpers.h"
#include "inputmanager.h"
#include "../text/textformatter.h"
#include "../rendering/renderviewport.h"
#include "../rendering/textmetrics.h"
#include "../rendering/textselectionrender.h"
#include "../text/incrementalformattedtext.h"

#include <string>
#include <memory>
#include <glm/vec2.hpp>
#include <GL/glew.h>

class Font;
struct RenderStyle;
class TextRender;
class Text;
struct InputState;

enum class FormatMode : std::uint8_t;

/**
 * The input state
 */
struct InputState {
	glm::vec2 viewPosition = glm::vec2();
	std::int64_t caretPositionX = 0;
	std::int64_t caretPositionY = 0;

	TextSelection selection;
};

using KeyboardCommandFunction = std::function<void ()>;

/**
 * Represents a keyboard command
 */
struct KeyboardCommand {
	int key;

	KeyboardCommandFunction normalMode =  {}; 		//When no modifier key is pressed
	KeyboardCommandFunction controlMode = {};  		//When control is being pressed
	KeyboardCommandFunction shiftMode = {};  		//When shift is being pressed
	KeyboardCommandFunction altMode = {};			//When alt is being pressed
};

/**
 * Defines how the formatting is performed
 */
enum class PerformFormattingType {
	Full,
	Partial,
	Incremental
};

/**
 * Defines how character entries are handled
 */
enum class CharacterInputType {
	Native, // Uses the native GLFW method
	Custom // Uses a custom implementation
};

/**
 * Represents a text view
 */
class TextView {
private:
	GLFWwindow* mWindow;

	PerformFormattingType mPerformFormattingType = PerformFormattingType::Incremental;
	CharacterInputType mCharacterInputType = CharacterInputType::Native;

	const Font& mFont;
	FormatMode mFormatMode;
	TextFormatter mTextFormatter;
	const RenderStyle& mRenderStyle;
	TextMetrics mTextMetrics;

	const RenderViewPort& mViewPort;

	InputManager mInputManager;
	InputState mInputState;
	std::vector<KeyboardCommand> mKeyboardCommands;
	const float mScrollSpeed = 4.0f;

	std::size_t mTextVersion = 0;
	Text& mText;
	std::unique_ptr<BaseFormattedText> mFormattedText;

	RenderViewPort mLastViewPort;
	bool mViewMoved = false;

	bool mDrawCaret = false;
	TimePoint mLastCaretUpdate;

	TextSelectionRender mTextSelectionRender;
	bool mSelectionStarted = false;
	TextSelection mPotentialSelection;
	bool mShowSelection = false;

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
	 * Returns the width of the current line
	 */
	float currentLineWidth() const;

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
	 * Sets the caret in the x position to the given value
	 * @param position The new value
	 */
	void setCaretX(std::int64_t position);

	/**
	 * Moves the caret in the y position by the given amount
	 * @param diff The amount to move
	 */
	void moveCaretY(std::int64_t diff);

	/**
	 * Clamps the view position y
	 * @param caretScreenPositionY The y position of the caret on the screen
	 */
	void clampViewPositionY(float caretScreenPositionY);

	/**
	 * Updates the input
	 * @param windowState The window state
	 */
	void updateInput(const WindowState& windowState);

	/**
	 * Returns the input state for incremental formatting
	 */
	IncrementalFormattedText::InputState getIncrementalFormattingInputState();

	/**
	 * Inserts the given character
	 * @param character The character
	 */
	void insertCharacter(Char character);

	/**
	 * The action for insertion
	 * @param character The character to insert
	 */
	void insertAction(Char character);

	/**
	 * Inserts a new line
	 */
	void insertLine();

	/**
	 * Pastes from the clipboard
	 */
	void paste();

	/**
	 * Deletes the current line
	 * @param mode How to delete the line
	 */
	void deleteLine(Text::DeleteLineMode mode);

	/**
	 * Deletes the current selection
	 */
	void deleteSelection();

	/**
	 * Deletes the given character
	 * @param lineIndex The line index
	 * @param charIndex The char index
	 */
	void deleteCharacter(std::size_t lineIndex, std::size_t charIndex);

	/**
	 * The action for the backspace button
	 */
	void backspaceAction();

	/**
	 * The action for the delete button
	 */
	void deleteAction();

	/**
	 * Replaces the current selection with the given character
	 * @param character The character
	 */
	void replaceSelection(Char character);

	/**
	 * Updates the editing
	 * @param windowState The window state
	 */
	void updateEditing(const WindowState& windowState);

	/**
	 * Moves the view by the given amount in the y direction
	 * @param diff The diff
	 */
	void moveViewY(float diff);

	/**
	 * Updates the view movement
	 * @param windowState The window state
	 */
	void updateViewMovement(const WindowState& windowState);

	/**
	 * Returns the position of the mouse in the text
	 */
	std::pair<std::int64_t, std::int64_t> getMouseTextPosition();

	/**
	 * Updates text selection
	 * @param windowState The window state
	 */
	void updateTextSelection(const WindowState& windowState);

	/**
	 * Updates the mouse movement
	 * @param windowState The window state
	 */
	void updateMouseMovement(const WindowState& windowState);

	/**
	 * Returns the spacing due to line numbers
	 */
	float getLineNumberSpacing() const;

	/**
	 * Returns the view port for the text part
	 */
	RenderViewPort getTextViewPort() const;

	/**
	 * Returns the draw position
	 */
	glm::vec2 getDrawPosition() const;

	/**
	 * Updates the formatted text
	 * @param viewPort The view port
	 */
	void updateFormattedText(const RenderViewPort& viewPort);

	/**
	 * Formats the given line for the given partial formatting
	 * @param viewPort The view port
	 * @param formattedText The formatted text
	 * @param lineIndex The line index
	 */
	void formatLinePartialMode(const RenderViewPort& viewPort, PartialFormattedText& formattedText, std::size_t lineIndex);

	/**
	 * Reformats the given line index
	 * @param lineIndex The index of the line
	 * @param formattedText The formatted text. If null, uses mFormattedText
	 */
	void reformatLine(std::size_t lineIndex, FormattedText* formattedText = nullptr);

	/**
	 * Reformats the given lines
	 * @param startLineIndex The index of the first line
	 * @param endLineIndex The index of the last line
	 * @param formattedText The formatted text. If null, uses mFormattedText
	 */
	void reformatLines(std::size_t startLineIndex, std::size_t endLineIndex, FormattedText* formattedText = nullptr);

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
	 * Returns the text
	 */
	Text& text();

	/**
	 * Updates the text view
	 * @param windowState The window state
	 */
	void update(const WindowState& windowState);

	/**
	 * Renders the current view
	 * @param windowState The window state
	 * @param textRender The text text render
	 */
	void render(const WindowState& windowState, TextRender& textRender);
};