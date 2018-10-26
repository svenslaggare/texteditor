#include "textoperations.h"
#include "textview.h"
#include "../rendering/renderstyle.h"
#include "../rendering/font.h"

float TextOperations::getLineNumberSpacing(const Font& font, const Text& text) {
	return (std::to_string(text.numLines()).size() + 1) * font['A'].advanceX;
}

TextOperations::TextOperations(PerformFormattingType performFormattingType,
							   Font& font,
							   std::unique_ptr<FormatterRules> formattingRules,
							   const RenderStyle& renderStyle,
							   Text& text,
							   InputState& inputState)
	: mPerformFormattingType(performFormattingType),
	  mFont(font),
	  mTextFormatter(std::move(formattingRules)),
	  mRenderStyle(renderStyle),
	  mText(text),
	  mInputState(inputState) {

}

BaseFormattedText* TextOperations::formattedText() const {
	return mFormattedText.get();
}

IncrementalFormattedText::InputState TextOperations::getIncrementalFormattingInputState() {
	return { (std::size_t)mInputState.caretLineIndex, (std::size_t)mInputState.caretCharIndex };
}

IncrementalFormattedText* TextOperations::incrementalFormattedText() {
	return (IncrementalFormattedText*)mFormattedText.get();
}

std::size_t TextOperations::numLines() {
	return mText.numLines();
}

void TextOperations::viewMoved() {
	mViewMoved = true;
}

void TextOperations::formatLinePartialMode(const RenderViewPort& viewPort, PartialFormattedText& formattedText, std::size_t lineIndex) {
	FormattedLine formattedLine;
	mTextFormatter.formatLine(mFont, mRenderStyle, viewPort, mText.getLine(lineIndex), formattedLine);
	formattedLine.number = lineIndex;
	formattedText.addLine(lineIndex, formattedLine);
}

void TextOperations::requireLineFormatted(const RenderViewPort& viewPort, std::size_t lineIndex) {
	if (mPerformFormattingType == PerformFormattingType::Partial) {
		auto* formattedText = (PartialFormattedText*)mFormattedText.get();
		if (!formattedText->hasLine(lineIndex)) {
			formatLinePartialMode(viewPort, *formattedText, lineIndex);
		}
	}
}

void TextOperations::requireSelectionFormatted(const RenderViewPort& viewPort, const TextSelection& selection) {
	if (mPerformFormattingType == PerformFormattingType::Partial) {
		auto* formattedText = (PartialFormattedText*)mFormattedText.get();

//		for (std::size_t lineIndex = mInputState.selection.startY; lineIndex <= mInputState.selection.endY; lineIndex++) {
//			if (!formattedText->hasLine((std::size_t)lineIndex)) {
//				formatLinePartialMode(viewPort, textFormatter, *formattedText, lineIndex);
//			}
//		}

		if (!formattedText->hasLine((std::size_t)mInputState.selection.startLine)) {
			formatLinePartialMode(viewPort, *formattedText, (std::size_t)mInputState.selection.startLine);
		}

		if (!formattedText->hasLine((std::size_t)mInputState.selection.endLine)) {
			formatLinePartialMode(viewPort, *formattedText, (std::size_t)mInputState.selection.endLine);
		}
	}
}

PartialFormattedText TextOperations::performPartialFormatting(const RenderViewPort& viewPort, glm::vec2 position) {
	PartialFormattedText formattedText;
	formattedText.setNumLines(mText.numLines());

	auto formatLine = [&](std::size_t lineIndex) {
		formatLinePartialMode(viewPort, formattedText, lineIndex);
	};

	if ((std::size_t)mInputState.caretLineIndex < mText.numLines()) {
		formatLine((std::size_t)mInputState.caretLineIndex);
	}

	auto cursorLineIndex = (std::int64_t)std::floor(-position.y / mFont.lineHeight());
	for (std::int64_t lineIndex = cursorLineIndex; lineIndex < (std::int64_t)mText.numLines(); lineIndex++) {
		if (lineIndex >= 0) {
			glm::vec2 drawPosition(position.x, position.y + (lineIndex + 1) * mFont.lineHeight());

			if (drawPosition.y >= viewPort.top()) {
				formatLine((std::size_t)lineIndex);
			}

			if (drawPosition.y > viewPort.bottom()) {
				break;
			}
		}
	}

	if (mInputState.selection.startLine != mInputState.selection.endLine && mInputState.showSelection) {
		formatLine(mInputState.selection.startLine);
		formatLine(std::min(mInputState.selection.endLine, numLines() - 1));
	}

	return formattedText;
}

void TextOperations::updateFormattedText(const RenderViewPort& viewPort) {
	bool needUpdate = mLastViewPort.width != viewPort.width
					  || mLastViewPort.height != viewPort.height
					  || mLastViewPort.position != viewPort.position
					  || mText.hasChanged(mTextVersion);

	if (mPerformFormattingType == PerformFormattingType::Partial) {
		needUpdate |= mViewMoved;
	}

	if (needUpdate) {
		mLastViewPort = viewPort;

		switch (mPerformFormattingType) {
			case PerformFormattingType::Full: {
				auto t0 = Helpers::timeNow();
				auto formattedText = std::make_unique<FormattedText>();
				mTextFormatter.format(mFont, mRenderStyle, viewPort, mText, formattedText->lines());
				mFormattedText = std::move(formattedText);
				std::cout
					<< "Formatted text (lines = " << numLines() << ") in "
					<< (Helpers::durationMicroseconds(Helpers::timeNow(), t0) / 1E3) << " ms"
					<< std::endl;

//				formattedBenchmark(mFont, mFormatMode, mRenderStyle, viewPort, mText);

				mInputState.caretLineIndex = std::min(mInputState.caretLineIndex, (std::int64_t)numLines() - 1);
				break;
			}
			case PerformFormattingType::Incremental: {
				auto t0 = Helpers::timeNow();
				mFormattedText = std::make_unique<IncrementalFormattedText>(
					mFont,
					mTextFormatter,
					mRenderStyle,
					viewPort,
					mText,
					mTextVersion);

				std::cout
					<< "Formatted text (lines = " << numLines() << ") in "
					<< (Helpers::durationMicroseconds(Helpers::timeNow(), t0) / 1E3) << " ms"
					<< std::endl;

//				formattedBenchmark(mFont, mTextFormatter, mRenderStyle, viewPort, mText);

				mInputState.caretLineIndex = std::min(mInputState.caretLineIndex, (std::int64_t) numLines() - 1);
				break;
			}
			case PerformFormattingType::Partial: {
				auto t0 = Helpers::timeNow();
				mViewMoved = false;
				mFormattedText = std::make_unique<PartialFormattedText>(performPartialFormatting(
					viewPort,
					mInputState.getDrawPosition(mRenderStyle)
					+ glm::vec2(TextOperations::getLineNumberSpacing(mFont, mText), 0.0f)));

				std::cout
					<< "Partial formatted text (lines = " << numLines() << ") in "
					<< (Helpers::durationMicroseconds(Helpers::timeNow(), t0) / 1E3) << " ms"
					<< std::endl;
				break;
			}
		}
	}
}

void TextOperations::insertCharacter(const RenderViewPort& viewPort, Char character) {
	mText.insertAt((std::size_t)mInputState.caretLineIndex, (std::size_t)mInputState.caretCharIndex, character);

	if (mPerformFormattingType == PerformFormattingType::Incremental) {
		incrementalFormattedText()->insertCharacter(getIncrementalFormattingInputState());
	} else {
		updateFormattedText(viewPort);
	}
}

void TextOperations::insertLine(const RenderViewPort& viewPort) {
	mText.splitLine((std::size_t)mInputState.caretLineIndex, (std::size_t)mInputState.caretCharIndex);

	if (mPerformFormattingType == PerformFormattingType::Incremental) {
		incrementalFormattedText()->insertLine(getIncrementalFormattingInputState());
	} else {
		updateFormattedText(viewPort);
	}
}

std::pair<std::size_t, std::size_t> TextOperations::paste(const RenderViewPort& viewPort, const String& text) {
	Text pasteText(text);

	auto diffCaretX = text.size();
	std::size_t diffCaretY = 0;

	if (pasteText.numLines() > 1) {
		mText.insertText((std::size_t)mInputState.caretLineIndex, (std::size_t)mInputState.caretCharIndex, pasteText);
		diffCaretX = pasteText.getLine(pasteText.numLines() - 1).size();
		diffCaretY = pasteText.numLines() - 1;
	} else {
		mText.insertAt((std::size_t)mInputState.caretLineIndex, (std::size_t)mInputState.caretCharIndex, text);
	}

	if (mPerformFormattingType == PerformFormattingType::Incremental) {
		incrementalFormattedText()->paste(
			getIncrementalFormattingInputState(),
			pasteText.numLines());
	} else {
		updateFormattedText(viewPort);
	}

	return std::make_pair(diffCaretX, diffCaretY);
}

void TextOperations::deleteLine(const RenderViewPort& viewPort, Text::DeleteLineMode mode) {
	auto diff = mText.deleteLine((std::size_t)mInputState.caretLineIndex, mode);
	if (mode == Text::DeleteLineMode::Start) {
		mInputState.caretCharIndex = diff.caretX;
	}

	if (mPerformFormattingType == PerformFormattingType::Incremental) {
		incrementalFormattedText()->deleteLine(getIncrementalFormattingInputState(), mode);
	} else {
		updateFormattedText(viewPort);
	}
}

void TextOperations::deleteSelection(const RenderViewPort& viewPort, const TextSelection& textSelection) {
	auto deleteData = mText.deleteSelection(mInputState.selection);

	if (mPerformFormattingType == PerformFormattingType::Incremental) {
		incrementalFormattedText()->deleteSelection(
			getIncrementalFormattingInputState(),
			textSelection,
			deleteData);
	} else {
		updateFormattedText(viewPort);
	}
}

void TextOperations::deleteCharacter(const RenderViewPort& viewPort, std::size_t charIndex) {
	mText.deleteAt((std::size_t)mInputState.caretLineIndex, charIndex);

	if (mPerformFormattingType == PerformFormattingType::Incremental) {
		incrementalFormattedText()->deleteCharacter(getIncrementalFormattingInputState());
	} else {
		updateFormattedText(viewPort);
	}
}