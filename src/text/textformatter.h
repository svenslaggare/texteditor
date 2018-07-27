#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "text.h"

class Font;
struct RenderViewPort;
struct RenderStyle;
class FormattedText;
class Text;

/**
 * The type of a token
 */
enum class TokenType {
	Text,
	Keyword,
	String,
	Comment
};

/**
 * Represents a token
 */
struct Token {
	TokenType type = TokenType::Text;
	String text;
};

/**
 * Represents the tokens for a text line
 */
struct LineTokens {
	std::size_t number = 0;
	std::vector<Token> tokens;
	std::size_t offsetFromTextLine = 0;

	bool isContinuation = false;

	/**
	 * Adds the given token
	 * @param token The token
	 */
	void addToken(Token token);

	/**
	 * Returns the number of characters on the line
	 */
	std::size_t length() const;
};

/**
 * Represents a base class for formatted text
 */
class BaseFormattedText {
public:
	virtual ~BaseFormattedText() = default;

	/**
	 * Returns the number of lines
	 */
	virtual std::size_t numLines() const = 0;

	/**
	 * Returns the tokens at the given line
	 * @param index The index
	 */
	virtual const LineTokens& getLine(std::size_t index) const = 0;
};

/**
 * Represents formatted text
 */
class FormattedText : public BaseFormattedText {
private:
	std::vector<LineTokens> mLines;
public:
	/**
	 * Returns the number of lines
	 */
	std::size_t numLines() const override;

	/**
	 * Returns the tokens at the given line
	 * @param index The index
	 */
	const LineTokens& getLine(std::size_t index) const override;

	/**
	 * Adds the given line
	 * @param tokens The tokens on the line
	 */
	void addLine(LineTokens tokens);
};

/**
 * Represents a partially formatted text
 */
class PartialFormattedText : public BaseFormattedText {
private:
	std::size_t mTotalLines;
	std::unordered_map<std::size_t, LineTokens> mLines;
public:
	/**
	 * Returns the number of lines
	 */
	std::size_t numLines() const override;

	/**
	 * Sets the number of lines
	 * @param count The total number of lines
	 */
	void setNumLines(std::size_t count);

	/**
	 * Returns the tokens at the given line
	 * @param index The index
	 */
	const LineTokens& getLine(std::size_t index) const override;

	/**
	 * Adds the given line
	 * @param index The index for the line
	 * @param tokens The tokens on the line
	 */
	void addLine(std::size_t index, LineTokens tokens);

	/**
	 * Indicates if the given line exists
	 * @param index The index
	 */
	bool hasLine(std::size_t index) const;
};

/**
 * The format mode
 */
enum class FormatMode : std::uint8_t {
	Text,
	Code
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
					LineTokens& formattedLine);

	/**
	 * Formats the given text using the given font
	 * @param font The font
	 * @param viewPort The view port to render to
	 * @param renderStyle The render style
 	 * @param text The text
	 * @param lines The lines
	 */
	void format(const Font& font,
				const RenderStyle& renderStyle,
				const RenderViewPort& viewPort,
				const Text& text,
				FormattedText& formattedText);
};