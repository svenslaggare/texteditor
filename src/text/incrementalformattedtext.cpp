#include "incrementalformattedtext.h"
#include "../helpers.h"

IncrementalFormattedText::IncrementalFormattedText(const Font& font,
												   const RenderStyle& renderStyle,
												   const RenderViewPort& viewPort,
												   Text& text,
												   std::size_t& textVersion,
												   FormatMode formatMode)
	: mFont(font),
	  mRenderStyle(renderStyle),
	  mViewPort(viewPort),
	  mTextFormatter(formatMode),
	  mText(text),
	  mTextVersion(textVersion) {
	mTextFormatter.format(mFont, mRenderStyle, mViewPort, mText, mFormattedText);
}

std::size_t IncrementalFormattedText::numLines() const {
	return mFormattedText.numLines();
}

const LineTokens& IncrementalFormattedText::getLine(std::size_t index) const {
	return mFormattedText.getLine(index);
}

void IncrementalFormattedText::reformatLine(std::size_t lineIndex) {
	LineTokens lineTokens;
	mTextFormatter.formatLine(mFont, mRenderStyle, mViewPort, mText.getLine(lineIndex), lineTokens);
	lineTokens.number = lineIndex;
	mFormattedText.lines()[lineIndex] = std::move(lineTokens);
}

void IncrementalFormattedText::reformatLines(std::size_t startLineIndex, std::size_t endLineIndex) {
	std::vector<LineTokens> formattedLines;
	std::vector<const String*> lines;
	for (std::size_t i = startLineIndex; i <= endLineIndex; i++) {
		lines.push_back(&mText.getLine(i));
		std::cout << Helpers::toString(*lines.back()) << std::endl;
	}

	mTextFormatter.formatLines(mFont, mRenderStyle, mViewPort, lines, formattedLines);

	for (std::size_t i = 0; i <= endLineIndex - startLineIndex; i++) {
		auto& formattedLine = formattedLines[i];
		formattedLine.number = startLineIndex + i;
		mFormattedText.lines()[startLineIndex + i] = std::move(formattedLine);
	}
}

void IncrementalFormattedText::insertCharacter(const InputState& inputState) {
	auto& currentFormattedLine = mFormattedText.getLine(inputState.lineIndex);
	auto& startSearchLine = mFormattedText.getLine((std::size_t)(inputState.lineIndex + currentFormattedLine.reformatStartSearch));

	auto reformatStart = startSearchLine.number;
	auto reformatEnd = startSearchLine.number + startSearchLine.reformatAmount;

	if (reformatStart == reformatEnd) {
		reformatLine(inputState.lineIndex);
	} else {
		reformatLines(reformatStart, reformatEnd);
	}

	mText.hasChanged(mTextVersion);
}

void IncrementalFormattedText::insertLine(const InputState& inputState) {
	reformatLine(inputState.lineIndex);

	LineTokens newLineTokens;
	mTextFormatter.formatLine(mFont, mRenderStyle, mViewPort, mText.getLine(inputState.lineIndex + 1), newLineTokens);
	mFormattedText.lines().insert(mFormattedText.lines().begin() + inputState.lineIndex + 1, newLineTokens);

	for (std::size_t i = inputState.lineIndex; i < mFormattedText.lines().size(); i++) {
		mFormattedText.lines()[i].number = i;
	}

	mText.hasChanged(mTextVersion);
}

void IncrementalFormattedText::paste(const InputState& inputState) {
	reformatLine(inputState.lineIndex);
	mText.hasChanged(mTextVersion);
}

void IncrementalFormattedText::deleteLine(const InputState& inputState, Text::DeleteLineMode mode) {
	auto lineNumber = inputState.lineIndex;

	if (mode == Text::DeleteLineMode::Start) {
		if (lineNumber > 0) {
			reformatLine(lineNumber - 1);
		}

		mFormattedText.lines().erase(mFormattedText.lines().begin() + lineNumber);

		for (std::size_t i = lineNumber - 1; i < mFormattedText.lines().size(); i++) {
			mFormattedText.lines()[i].number = i;
		}
	} else {
		if (lineNumber + 1 < mFormattedText.lines().size()) {
			reformatLine(lineNumber);
			mFormattedText.lines().erase(mFormattedText.lines().begin() + lineNumber + 1);

			for (std::size_t i = lineNumber; i < mFormattedText.lines().size(); i++) {
				mFormattedText.lines()[i].number = i;
			}
		}
	}

	mText.hasChanged(mTextVersion);
}

void IncrementalFormattedText::deleteSelection(const InputState& inputState, const TextSelection& textSelection, const Text::DeleteSelectionData& deleteData) {
	if (textSelection.startY == textSelection.endY) {
		reformatLine(textSelection.startY);
	} else {
		mFormattedText.lines().erase(
			mFormattedText.lines().begin() + deleteData.startDeleteLineIndex,
			mFormattedText.lines().begin() + deleteData.endDeleteLineIndex + 1);

		reformatLine(textSelection.startY);
		reformatLine(textSelection.startY + 1);

		for (std::size_t i = textSelection.startY; i < mFormattedText.lines().size(); i++) {
			mFormattedText.lines()[i].number = i;
		}
	}

	mText.hasChanged(mTextVersion);
}

void IncrementalFormattedText::deleteCharacter(const InputState& inputState) {
//		reformatLine(lineIndex);
	auto& currentFormattedLine = mFormattedText.getLine(inputState.lineIndex);
	auto& startSearchLine = mFormattedText.getLine((std::size_t)(inputState.lineIndex + currentFormattedLine.reformatStartSearch));

	auto reformatStart = startSearchLine.number;
	auto reformatEnd = startSearchLine.number + startSearchLine.reformatAmount;

	if (reformatStart == reformatEnd) {
		reformatLine(inputState.lineIndex);
	} else {
		reformatLines(reformatStart, reformatEnd);
	}

	mText.hasChanged(mTextVersion);
}
