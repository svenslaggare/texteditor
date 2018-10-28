#pragma once
#include "text.h"
#include "formattedtext.h"
#include "helpers.h"
#include "formatterrules.h"

#include <string>
#include <vector>
#include <iostream>

class Font;
struct RenderViewPort;
struct RenderStyle;
class Text;

using FormattedLines = std::vector<FormattedLine>;

/**
 * The state of the text formatter
 */
enum class State {
	Text,
	String,
	Number,
	Comment,
	BlockComment,
};

/**
 * The internal formatter state machine
 */
class FormatterStateMachine {
private:
	const FormatterRules& mRules;

	const Font& mFont;
	const RenderStyle& mRenderStyle;
	const RenderViewPort& mViewPort;
	FormattedLines& mFormattedLines;

	std::size_t mLineNumber = 0;
	std::size_t mBlockCommentStartIndex = 0;

	State mState = State::Text;
	bool mIsWhitespace = false;
	bool mIsEscaped = false;
	Char mStringStartDelimiter;

	FormattedLine mCurrentFormattedLine;
	Token mCurrentToken;
	float mCurrentWidth = 0.0f;

	CircularBuffer<Char> mPrevCharBuffer;

	String getPrevChars(std::size_t size);
	void removeChars(std::size_t count);
	bool isPrevCharsMatch(const String& string, Char current, String& prevChars);

	void tryMakeKeyword();
	void newToken(TokenType type = TokenType::Text, bool makeKeyword = false);
	void addChar(Char character, float advanceX);

	void handleTab();

	void handleText(Char current, float advanceX);
	void handleString(Char current, float advanceX);
	void handleNumber(Char current, float advanceX);
	void handleComment(Char current, float advanceX);
	void handleBlockComment(Char current, float advanceX);
public:
	FormatterStateMachine(const FormatterRules& textFormatterRules,
						  const Font& font,
						  const RenderStyle& renderStyle,
						  const RenderViewPort& viewPort,
						  FormattedLines& formattedLines);

	State state() const;
	const FormattedLine& currentFormattedLine() const;

	void createNewLine(bool resetState = true, bool continueWithLine = false, bool allowKeyword = true);
	void processCodeMode(Char current);
	void processTextMode(Char current);
	void process(Char current);

	void processLine(const String& line);
};

/**
 * Represents a text formatter
 */
class TextFormatter {
private:
	std::unique_ptr<FormatterRules> mRules;
public:
	/**
	 * Creates a new text formatter
	 * @param rules The formatting rules
	 */
	explicit TextFormatter(std::unique_ptr<FormatterRules> rules);

	/**
	 * Returns the formatting rules
	 */
	const FormatterRules& rules() const;

	/**
	 * Creates a new formatter state machine
	 * @param font The font
	 * @param renderStyle The render style
	 * @param viewPort The view port
	 * @param formattedLines The formatted lines
	 */
	FormatterStateMachine createStateMachine(const Font& font,
											 const RenderStyle& renderStyle,
											 const RenderViewPort& viewPort,
											 FormattedLines& formattedLines);

	/**
	 * Formats the given line
	 * @param font The font
	 * @param viewPort The view port to render to
	 * @param renderStyle The render style
	 * @param line The line to format
	 * @param formattedLine The formatted line
	 */
	void formatLine(const Font& font,
					const RenderStyle& renderStyle,
					const RenderViewPort& viewPort,
					const String& line,
					FormattedLine& formattedLine);

	/**
	 * Formats the given text using the given font
	 * @param font The font
	 * @param viewPort The view port to render to
	 * @param renderStyle The render style
 	 * @param text The text
 	 * @param formattedLines The formatted lines
	 */
	void format(const Font& font,
				const RenderStyle& renderStyle,
				const RenderViewPort& viewPort,
				const Text& text,
				FormattedLines& formattedLines);
};