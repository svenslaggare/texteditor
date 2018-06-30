#pragma once

#include <string>
#include <vector>
#include <iostream>

class Font;
class RenderViewPort;
class RenderStyle;
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
	std::string text;
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
 * Represents formatted text
 */
class FormattedText {
private:
	std::vector<LineTokens> mLines;
public:
	/**
	 * Returns the number of lines
	 */
	std::size_t numLines() const;

	/**
	 * Returns the tokens at the given line
	 * @param index The index
	 */
	const LineTokens& getLine(std::size_t index) const;

	/**
	 * Adds the given line
	 * @param tokens The tokens on the line
	 */
	void addLine(LineTokens tokens);
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