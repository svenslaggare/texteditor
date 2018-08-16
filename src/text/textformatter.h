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
 * The internal formatter state machine
 */
class FormatterStateMachine {
private:
	FormatMode mMode;
	const Font& mFont;
	const RenderStyle& mRenderStyle;
	const RenderViewPort& mViewPort;
	FormattedLines& mFormattedLines;

	std::size_t mLineNumber = 0;
	std::size_t mBlockCommentStartIndex = 0;

	State mState = State::Text;
	bool mIsWhitespace = false;
	bool mIsEscaped = false;

	FormattedLine mCurrentFormattedLine;
	Token mCurrentToken;
	float mCurrentWidth = 0.0f;

	CircularBuffer<Char> mPrevCharBuffer;

	String mLineCommentStart = u"//";
	String mBlockCommentStart = u"/*";
	String mBlockCommentEnd = u"*/";

	String getPrevChars(std::size_t size);

	void removeChars(std::size_t count);

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
	FormatterStateMachine(FormatMode mode,
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
 * The rules formatting
 */
class TextFormatterRules {
	virtual ~TextFormatterRules() = default;

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
};

/**
 * Represents a text formatter
 */
class TextFormatter {
private:
	FormatMode mMode;
public:
	/**
	 * Creates a new text formatter
	 * @param mode The format mode
	 */
	explicit TextFormatter(FormatMode mode = FormatMode::Code);

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