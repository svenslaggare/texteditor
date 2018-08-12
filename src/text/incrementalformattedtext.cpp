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
	  mFormatMode(formatMode),
	  mTextFormatter(formatMode),
	  mText(text),
	  mTextVersion(textVersion) {
	mTextFormatter.format(mFont, mRenderStyle, mViewPort, mText, mFormattedText);
}

std::size_t IncrementalFormattedText::numLines() const {
	return mFormattedText.numLines();
}

const FormattedLine& IncrementalFormattedText::getLine(std::size_t index) const {
	return mFormattedText.getLine(index);
}

namespace {
	void processLine(FormatterStateMachine& stateMachine, const String& line) {
		for (auto current : line) {
			stateMachine.process(current);
		}

		stateMachine.process('\n');
	}

	void formatUntilStateRestored(FormatterStateMachine& stateMachine, const Text& text, std::size_t startLineIndex, std::size_t& numFormattedLines) {
		for (std::size_t i = startLineIndex; i < text.numLines(); i++) {
			processLine(stateMachine, text.getLine(i));
			numFormattedLines++;

			if (stateMachine.state() == State::Text) {
				break;
			}
		}
	}
}

void IncrementalFormattedText::reformatLine(std::size_t lineIndex) {
//	FormattedLine formattedLine;
//	mTextFormatter.formatLine(mFont, mRenderStyle, mViewPort, mText.getLine(lineIndex), formattedLine);
//	formattedLine.number = lineIndex;
//	mFormattedText.lines()[lineIndex] = std::move(formattedLine);

	auto& currentFormattedLine = mFormattedText.getLine(lineIndex);
	auto& startSearchLine = mFormattedText.getLine((std::size_t)(lineIndex + currentFormattedLine.reformatStartSearch));

	auto reformatStart = startSearchLine.number;
	auto reformatEnd = startSearchLine.number + startSearchLine.reformatAmount;

	if (reformatStart == reformatEnd) {
		FormattedText formattedText;
		FormatterStateMachine stateMachine(mFormatMode, mFont, mRenderStyle, mViewPort, formattedText);

		processLine(stateMachine, mText.getLine(lineIndex));

		std::size_t numFormattedLines = 1;
		if (formattedText.getLine(0).mayRequireSearch && stateMachine.state() != State::Text) {
			std::cout << "reformatLine: mult" << std::endl;
			formatUntilStateRestored(stateMachine, mText, lineIndex + 1, numFormattedLines);
		} else {
			std::cout << "reformatLine: single" << std::endl;
		}

		if (!stateMachine.currentFormattedLine().tokens.empty()) {
			stateMachine.createNewLine();
		}

		for (std::size_t i = 0; i < numFormattedLines; i++) {
			auto formattedLine = formattedText.getLine(i);
			formattedLine.number = lineIndex + i;
			mFormattedText.lines()[lineIndex + i] = std::move(formattedLine);
		}
	} else {
		reformatLines(reformatStart, reformatEnd);
	}
}

void IncrementalFormattedText::reformatLines(std::size_t startLineIndex, std::size_t endLineIndex) {
	std::cout << "reformatLines" << std::endl;
//	std::vector<FormattedLine> formattedLines;
//	std::vector<const String*> lines;
//	for (std::size_t i = startLineIndex; i <= endLineIndex; i++) {
//		lines.push_back(&mText.getLine(i));
//	}
//
//	mTextFormatter.formatLines(mFont, mRenderStyle, mViewPort, lines, formattedLines);
//
//	for (std::size_t i = 0; i <= endLineIndex - startLineIndex; i++) {
//		auto& formattedLine = formattedLines[i];
//		formattedLine.number = startLineIndex + i;
//		mFormattedText.lines()[startLineIndex + i] = std::move(formattedLine);
//	}

	FormattedText formattedText;
	FormatterStateMachine stateMachine(mFormatMode, mFont, mRenderStyle, mViewPort, formattedText);

	std::size_t numFormattedLines = 0;
	bool continueFormatting = false;
	for (std::size_t i = startLineIndex; i <= endLineIndex; i++) {
		processLine(stateMachine, mText.getLine(i));
		numFormattedLines++;

		if (i == endLineIndex) {
			if (stateMachine.state() != State::Text) {
				continueFormatting = true;
			}
		}
	}

	if (continueFormatting) {
		formatUntilStateRestored(stateMachine, mText, endLineIndex + 1, numFormattedLines);
	}

	if (!stateMachine.currentFormattedLine().tokens.empty()) {
		stateMachine.createNewLine();
	}

	for (std::size_t i = 0; i < numFormattedLines; i++) {
		auto formattedLine = formattedText.getLine(i);
		formattedLine.number = startLineIndex + i;
		mFormattedText.lines()[startLineIndex + i] = std::move(formattedLine);
	}
}

void IncrementalFormattedText::reformatCharacterAction(const IncrementalFormattedText::InputState& inputState) {
	reformatLine(inputState.lineIndex);
	mText.hasChanged(mTextVersion);
}

void IncrementalFormattedText::insertCharacter(const InputState& inputState) {
	reformatCharacterAction(inputState);
}

void IncrementalFormattedText::insertLine(const InputState& inputState) {
	FormattedLine newFormattedLine;
	mTextFormatter.formatLine(mFont, mRenderStyle, mViewPort, mText.getLine(inputState.lineIndex + 1), newFormattedLine);
	mFormattedText.lines().insert(mFormattedText.lines().begin() + inputState.lineIndex + 1, newFormattedLine);
	reformatLine(inputState.lineIndex);

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
		mFormattedText.lines().erase(mFormattedText.lines().begin() + lineNumber);

		if (lineNumber > 0) {
			reformatLine(lineNumber - 1);
		}

		for (std::size_t i = lineNumber - 1; i < mFormattedText.lines().size(); i++) {
			mFormattedText.lines()[i].number = i;
		}
	} else {
		if (lineNumber + 1 < mFormattedText.lines().size()) {
			mFormattedText.lines().erase(mFormattedText.lines().begin() + lineNumber + 1);
			reformatLine(lineNumber);

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
	reformatCharacterAction(inputState);
}
