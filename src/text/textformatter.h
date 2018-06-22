#pragma once

#include <string>
#include <vector>

class Font;
class RenderViewPort;
class RenderStyle;
class FormattedText;

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