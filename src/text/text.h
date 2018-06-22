#pragma once
#include "../rendering/renderviewport.h"

#include <string>
#include <vector>

class Font;
class RenderStyle;

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
struct FormattedText {
	std::vector<std::vector<Token>> lines;
};

enum class FormatMode : std::uint8_t;

/**
 * Represents text
 */
class Text {
private:
	std::string mRaw;
	std::size_t mNumLines = 0;

	mutable FormattedText mFormattedText;
	mutable RenderViewPort mLastViewPort;
public:
	/**
	 * Creates a new text
	 * @param text The raw text
	 */
	Text(std::string text);

	/**
	 * Returns the number of lines
	 */
	std::size_t numLines() const;

	/**
	 * Returns a formatted version of the current text
	 * @param formatMode The format mode
	 * @param font The font
	 * @param renderStyle The render style
	 * @param viewPort The view port
	 */
	const FormattedText& getFormatted(const Font& font,
									  FormatMode formatMode,
									  const RenderStyle& renderStyle,
									  const RenderViewPort& viewPort) const;
};