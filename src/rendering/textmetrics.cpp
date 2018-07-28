#include "textmetrics.h"
#include "renderstyle.h"

#include "../text/textformatter.h"

TextMetrics::TextMetrics(const Font& font, const RenderStyle& renderStyle)
	: mFont(font), mRenderStyle(renderStyle) {

}

float TextMetrics::calculatePositionX(const BaseFormattedText& text, std::size_t lineIndex, std::size_t offset) const {
	float lineOffset = 0.0f;
	std::size_t currentIndex = 0;
	for (auto& token : text.getLine(lineIndex).tokens) {
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

std::size_t TextMetrics::getCharIndexFromScreenPosition(const BaseFormattedText& text,
														std::size_t lineIndex,
														float screenPositionX) const {
	float currentCharOffset = 0.0f;
	std::size_t currentCharIndex = 0;
	for (auto& token : text.getLine(lineIndex).tokens) {
		for (auto character : token.text) {
			auto advanceX = mRenderStyle.getAdvanceX(mFont, character);

			if (screenPositionX >= currentCharOffset && screenPositionX <= currentCharOffset + advanceX) {
				return currentCharIndex;
			}

			currentCharOffset += advanceX;
			currentCharIndex++;
		}
	}

	return currentCharIndex;
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
				return lineWidth;
			}
		}
	}

	return lineWidth;
}
