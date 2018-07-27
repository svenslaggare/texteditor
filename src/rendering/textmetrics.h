#pragma once
#include <cstddef>

struct LineTokens;
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
	 * @param lineNumber The line number in the text
	 * @param offset The offset within the line
	 */
	float calculatePositionX(const BaseFormattedText& text,
							 std::size_t lineNumber,
							 std::size_t offset) const;

	/**
	 * Returns the width of the given line
	 * @param lineTokens The line tokens
	 * @param startCharIndex The start character index to use on the line
	 * @param maxCharIndex The maximum character index to use on the line
	 */
	float getLineWidth(const LineTokens& lineTokens,
					   std::size_t startCharIndex = 0,
					   std::size_t* maxCharIndex = nullptr) const;
};