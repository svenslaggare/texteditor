#pragma once

#include <string>
#include <vector>
#include <iostream>

class Font;
class RenderViewPort;
class RenderStyle;
class FormattedText;

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
 * Represents formatted text
 */
class FormattedText {
private:
	std::vector<std::vector<Token>> mLines;
public:
	/**
	 * Returns the number of lines
	 */
	std::size_t numLines() const;

	/**
	 * Returns the tokens at the given line
	 * @param index The index
	 */
	const std::vector<Token>& getLine(std::size_t index) const;

	/**
	 * Adds the given line
	 * @param tokens The tokens on the line
	 */
	void addLine(std::vector<Token> tokens);
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
				const std::string& text,
				FormattedText& formattedText);
};