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
	mTextFormatter.format(mFont, mRenderStyle, mViewPort, mText, mFormattedLines);
}

std::size_t IncrementalFormattedText::numLines() const {
	return mFormattedLines.size();
}

const FormattedLine& IncrementalFormattedText::getLine(std::size_t index) const {
	return mFormattedLines[index];
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
	auto& currentFormattedLine = mFormattedLines[lineIndex];
	auto& startSearchLine = mFormattedLines[(std::size_t)(lineIndex + currentFormattedLine.reformatStartSearch)];

	auto reformatStart = startSearchLine.number;
	auto reformatEnd = startSearchLine.number + startSearchLine.reformatAmount;

	if (reformatStart == reformatEnd) {
		FormattedLines formattedLines;
		FormatterStateMachine stateMachine(mFormatMode, mFont, mRenderStyle, mViewPort, formattedLines);

		processLine(stateMachine, mText.getLine(lineIndex));

		std::size_t numFormattedLines = 1;
		if (formattedLines.front().mayRequireSearch && stateMachine.state() != State::Text) {
//			std::cout << "reformatLine: mult" << std::endl;
			formatUntilStateRestored(stateMachine, mText, lineIndex + 1, numFormattedLines);
		} else {
//			std::cout << "reformatLine: single" << std::endl;
		}

		if (!stateMachine.currentFormattedLine().tokens.empty()) {
			stateMachine.createNewLine();
		}

		for (std::size_t i = 0; i < numFormattedLines; i++) {
			auto formattedLine = formattedLines[i];
			formattedLine.number = lineIndex + i;
			mFormattedLines[lineIndex + i] = std::move(formattedLine);
		}
	} else {
		reformatLines(reformatStart, reformatEnd);
	}
}

void IncrementalFormattedText::reformatLines(std::size_t startLineIndex, std::size_t endLineIndex) {
	// The line before may include multi-line parsing, include if the reformatting if needed
	if (startLineIndex >= 1) {
		auto prevLineIndex = startLineIndex - 1;
		auto& currentFormattedLine = mFormattedLines[prevLineIndex];
		auto& startSearchLine = mFormattedLines[(std::size_t)(prevLineIndex + currentFormattedLine.reformatStartSearch)];

		auto reformatStart = startSearchLine.number;
		auto reformatEnd = startSearchLine.number + startSearchLine.reformatAmount;

		if (reformatStart != reformatEnd) {
			startLineIndex = std::min(startLineIndex, reformatStart);
			endLineIndex = std::max(endLineIndex, reformatEnd);
		}
	}

	FormattedLines formattedLines;
	FormatterStateMachine stateMachine(mFormatMode, mFont, mRenderStyle, mViewPort, formattedLines);

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
		auto formattedLine = formattedLines[i];
		formattedLine.number = startLineIndex + i;
		mFormattedLines[startLineIndex + i] = std::move(formattedLine);
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
	mFormattedLines.insert(mFormattedLines.begin() + inputState.lineIndex + 1, newFormattedLine);
	reformatLine(inputState.lineIndex);

	for (std::size_t i = inputState.lineIndex; i < mFormattedLines.size(); i++) {
		mFormattedLines[i].number = i;
	}

	mText.hasChanged(mTextVersion);
}

void IncrementalFormattedText::paste(const InputState& inputState, std::size_t numLines) {
	if (numLines > 1) {
		mFormattedLines.insert(mFormattedLines.begin() + inputState.lineIndex, numLines - 1, FormattedLine {});
		reformatLines(inputState.lineIndex, inputState.lineIndex + numLines - 1);

		for (std::size_t i = inputState.lineIndex; i < mFormattedLines.size(); i++) {
			mFormattedLines[i].number = i;
		}
	} else {
		reformatLine(inputState.lineIndex);
	}

	mText.hasChanged(mTextVersion);
}

void IncrementalFormattedText::deleteLine(const InputState& inputState, Text::DeleteLineMode mode) {
	auto lineNumber = inputState.lineIndex;

	if (mode == Text::DeleteLineMode::Start) {
		mFormattedLines.erase(mFormattedLines.begin() + lineNumber);

		if (lineNumber > 0) {
			reformatLine(lineNumber - 1);
		}

		for (std::size_t i = lineNumber - 1; i < mFormattedLines.size(); i++) {
			mFormattedLines[i].number = i;
		}
	} else {
		if (lineNumber + 1 < mFormattedLines.size()) {
			mFormattedLines.erase(mFormattedLines.begin() + lineNumber + 1);
			reformatLine(lineNumber);

			for (std::size_t i = lineNumber; i < mFormattedLines.size(); i++) {
				mFormattedLines[i].number = i;
			}
		}
	}

	mText.hasChanged(mTextVersion);
}

void IncrementalFormattedText::deleteSelection(const InputState& inputState, const TextSelection& textSelection, const Text::DeleteSelectionData& deleteData) {
	if (textSelection.startY == textSelection.endY) {
		reformatLine(textSelection.startY);
	} else {
		mFormattedLines.erase(
			mFormattedLines.begin() + deleteData.startDeleteLineIndex,
			mFormattedLines.begin() + deleteData.endDeleteLineIndex + 1);

		reformatLine(textSelection.startY);
		reformatLine(textSelection.startY + 1);

		for (std::size_t i = textSelection.startY; i < mFormattedLines.size(); i++) {
			mFormattedLines[i].number = i;
		}
	}

	mText.hasChanged(mTextVersion);
}

void IncrementalFormattedText::deleteCharacter(const InputState& inputState) {
	reformatCharacterAction(inputState);
}
