#include "textmetrics.h"
#include "renderstyle.h"

#include "../text/textformatter.h"

TextMetrics::TextMetrics(const Font& font, const RenderStyle& renderStyle)
	: mFont(font), mRenderStyle(renderStyle) {

}

float TextMetrics::calculatePositionX(const BaseFormattedText& text, std::size_t lineNumber, std::size_t offset) const {
	float lineOffset = 0.0f;
	std::size_t currentIndex = 0;
	for (auto& token : text.getLine(lineNumber).tokens) {
		for (auto character : token.text) {
			if (currentIndex == offset) {
				return lineOffset;
			}

			lineOffset += mRenderStyle.getAdvanceX(mFont, character);
			currentIndex++;
		}
	}

	return lineOffset;
}

float TextMetrics::getLineWidth(const LineTokens& lineTokens, size_t startCharIndex, size_t* maxCharIndex) const {
	float lineWidth = 0.0f;
	std::size_t charIndex = 0;
	for (auto& token : lineTokens.tokens) {
		for (auto character : token.text) {
			auto advanceX = mRenderStyle.getAdvanceX(mFont, character);

			if (charIndex >= startCharIndex) {
				lineWidth += advanceX;
			}

			charIndex++;
			if (maxCharIndex != nullptr && charIndex > *maxCharIndex) {
				break;
			}
		}
	}

	return lineWidth;
}
