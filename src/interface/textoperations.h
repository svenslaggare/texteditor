#pragma once
#include <cstdint>
#include <utility>
#include <memory>

#include "../text/formattedtext.h"
#include "../rendering/renderviewport.h"
#include "../rendering/textmetrics.h"
#include "../text/textformatter.h"
#include "../text/incrementalformattedtext.h"

enum class PerformFormattingType : std::uint32_t;
struct InputState;

using LineAndOffset = std::pair<std::size_t, std::int64_t>;

/**
 * Applies operations to a text resulting in a formatted text
 */
class TextOperations {
private:
	PerformFormattingType mPerformFormattingType;

	Font& mFont;
	TextFormatter mTextFormatter;
	const RenderStyle& mRenderStyle;

	std::size_t mTextVersion = 0;
	Text& mText;

	std::unique_ptr<BaseFormattedText> mFormattedText;

	RenderViewPort mLastViewPort;
	bool mViewMoved = false;

	InputState& mInputState;

	/**
	 * Returns the formatting state for incremental formatting
	 * @param lineAndOffset The line and offset
	 */
	IncrementalFormattedText::InputState getIncrementalFormattingInputState(LineAndOffset lineAndOffset);

	/**
	 * Returns the incremental formatted text
	 */
	IncrementalFormattedText* incrementalFormattedText();

	/**
	 * Returns the number of lines in the text
	 */
	std::size_t numLines();

	/**
	 * Performs partial formatting at the given position
	 * @param viewPort The view port
	 * @param position The position
	 */
	PartialFormattedText performPartialFormatting(const RenderViewPort& viewPort, glm::vec2 position);

	/**
	 * Formats the given line in partial mode
	 * @param viewPort The view port
	 * @param formattedText The formatted text
	 * @param lineIndex The index of the line to format
	 */
	void formatLinePartialMode(const RenderViewPort& viewPort,
							   PartialFormattedText& formattedText,
							   std::size_t lineIndex);
public:
	/**
	 * Creates new text operations for the given text
	 * @param performFormattingType How the formatting will be performed
	 * @param font The font
	 * @param formattingRules The formatting rules
	 * @param renderStyle The render style
	 * @param text The input text
	 * @param inputState The input state
	 */
	TextOperations(PerformFormattingType performFormattingType,
				   Font& font,
				   std::unique_ptr<FormatterRules> formattingRules,
				   const RenderStyle& renderStyle,
				   Text& text,
				   InputState& inputState);

	/**
	 * Returns the spacing due to line numbers
	 * @param font The font
	 * @param text The text
	 */
	static float getLineNumberSpacing(const Font& font, const Text& text);

	/**
	 * Returns the formatted text
	 */
	BaseFormattedText* formattedText() const;

	/**
	 * Marks that the view moved
	 */
	void viewMoved();

	/**
	 * Updates the formatted text
	 * @param viewPort The view port
	 */
	void updateFormattedText(const RenderViewPort& viewPort);

	/**
	 * Requires that the given line to be formatted
	 * @param viewPort The view port
	 * @param lineIndex The index of the line
	 */
	void requireLineFormatted(const RenderViewPort& viewPort, std::size_t lineIndex);

	/**
	 * Requires that the given selection is formatted
	 * @param viewPort The view port
	 * @param selection The selection
	 */
	void requireSelectionFormatted(const RenderViewPort& viewPort, const TextSelection& selection);

	/**
	 * Inserts the given character
	 * @param lineAndOffset The line and offset
	 * @param viewPort The view port
	 * @param character The character to insert
	 */
	void insertCharacter(const RenderViewPort& viewPort, LineAndOffset lineAndOffset, Char character);

	/**
	 * Inserts a line at the given position
	 * @param lineAndOffset The line and offset
	 * @param viewPort The view port
	 */
	void insertLine(const RenderViewPort& viewPort, LineAndOffset lineAndOffset);

	/**
	 * Pastes the given text
	 * @param lineAndOffset The line and offset
	 * @param viewPort The view port
	 * @param text The text to paste
	 * @return Diff caret x, diff caret y
	 */
	std::pair<std::size_t, std::size_t> paste(const RenderViewPort& viewPort,
											  LineAndOffset lineAndOffset,
											  const String& text);

	/**
	 * Deletes the given line
	 * @param lineAndOffset The line and offset
	 * @param viewPort The view port
	 * @param mode The delete mode
	 */
	void deleteLine(const RenderViewPort& viewPort, LineAndOffset lineAndOffset, Text::DeleteLineMode mode);

	/**
	 * Deletes the given selection
	 * @param lineAndOffset The line and offset
	 * @param viewPort The view port
	 * @param textSelection The selection to delete
	 */
	void
	deleteSelection(const RenderViewPort& viewPort, LineAndOffset lineAndOffset, const TextSelection& textSelection);

	/**
	 * Deletes the given character
	 * @param lineAndOffset The line and offset
	 * @param viewPort The view port
	 * @param lineIndex The line index of the character
	 * @param charIndex The index of the character at the line
	 */
	void deleteCharacter(const RenderViewPort& viewPort,
						 LineAndOffset lineAndOffset,
						 std::size_t lineIndex,
						 std::size_t charIndex);
};