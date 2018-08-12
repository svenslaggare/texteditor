#pragma once
#include <cstddef>

struct FormattedLine;
class Font;
struct RenderStyle;
class BaseFormattedText;

/**
 * Represents a text metrics calculator
 */
class TextMetrics {
private:
	const Font& mFont;
	const RenderStyle& mRenderStyle;
public:
	/**
	 * Creates a new metrics calculator
	 * @param font The font
	 * @param renderStyle The render style
	 */
	TextMetrics(const Font& font, const RenderStyle& renderStyle);

	/**
	 * Calculates the X position of the given index at the given line
	 * @param text The text
	 * @param lineIndex The line index
	 * @param offset The offset within the line
	 */
	float calculatePositionX(const BaseFormattedText& text,
							 std::size_t lineIndex,
							 std::size_t offset) const;

	/**
	 * Returns the index of the character that the given screen position points to
	 * @param text The text
	 * @param lineIndex The line index
	 * @param screenPositionX The X screen position
	 */
	std::size_t getCharIndexFromScreenPosition(const BaseFormattedText& text,
											   std::size_t lineIndex,
											   float screenPositionX) const;


	/**
	 * Returns the width of the given line
	 * @param formattedLine The formatted line
	 * @param startCharIndex The start character index to use on the line
	 * @param maxCharIndex The maximum character index to use on the line
	 */
	float getLineWidth(const FormattedLine& formattedLine,
					   std::size_t startCharIndex = 0,
					   std::size_t* maxCharIndex = nullptr) const;
};