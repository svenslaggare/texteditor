#pragma once
#include "text.h"
#include "formattedtext.h"
#include "helpers.h"

#include <string>
#include <vector>
#include <iostream>

class Font;
struct RenderViewPort;
struct RenderStyle;

class Text;

/**
 * The format mode
 */
enum class FormatMode : std::uint8_t {
	Text,
	Code
};


enum class State {
	Text,
	String,
	Comment,
	BlockComment,
};

using FormattedLines = std::vector<FormattedLine>;

/**
 * The rules formatting
 */
class FormatterRules {
public:
	virtual ~FormatterRules() = default;

	/**
	 * Returns the mode
	 */
	virtual FormatMode mode() const = 0;

	/**
	 * Indicates if the given string is a keyword
	 * @param string The string
	 */
	virtual bool isKeyword(const String& string) const = 0;

	/**
	 * The start of a line comment
	 */
	virtual const String& lineCommentStart() const = 0;

	/**
	 * The start of a block comment
	 */
	virtual const String& blockCommentStart() const = 0;

	/**
	 * The end of a block comment
	 */
	virtual const String& blockCommentEnd() const = 0;

	/**
	 * Indicates if the given char is a string delimiter
	 * @param current The character 
	 */
	virtual bool isStringDelimiter(Char current) const = 0;
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
	void handleComment(Char current, float advanceX);
	void handleBlockComment(Char current, float advanceX);

	void handleCodeMode(Char current, float advanceX);
	void handleTextMode(Char current, float advanceX);
public:
	FormatterStateMachine(const FormatterRules& textFormatterRules,
						  const Font& font,
						  const RenderStyle& renderStyle,
						  const RenderViewPort& viewPort,
						  FormattedLines& formattedLines);

	State state() const;
	const FormattedLine& currentFormattedLine() const;

	void createNewLine(bool resetState = true, bool continueWithLine = false, bool allowKeyword = true);
	void process(Char current);
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
	 * @param rules The rules
	 */
	explicit TextFormatter( std::unique_ptr<FormatterRules> rules);

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
	 * Formats the given lines
	 * @param font The font
	 * @param viewPort The view port to render to
	 * @param renderStyle The render style
	 * @param lines The liens to format
	 * @param formattedLines The formatted lines
	 */
	void formatLines(const Font& font,
					 const RenderStyle& renderStyle,
					 const RenderViewPort& viewPort,
					 const std::vector<const String*>& lines,
					 FormattedLines& formattedLines);

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