#include "textview.h"
#include "../rendering/textrender.h"
#include "../rendering/renderstyle.h"
#include "../rendering/renderviewport.h"
#include "../rendering/font.h"

#include "../text/text.h"
#include "inputmanager.h"
#include "../windowstate.h"

#include <chrono>
#include <iostream>
#include <algorithm>
#include <memory>

namespace {
	Char convertCodePointToChar(CodePoint codePoint) {
		return (Char)codePoint;
	}
}

TextView::TextView(GLFWwindow* window,
				   const Font& font,
				   FormatMode formatMode,
				   const RenderViewPort& viewPort,
				   const RenderStyle& renderStyle,
				   Text& text)
	: mWindow(window),
	  mFont(font),
	  mFormatMode(formatMode),
	  mViewPort(viewPort),
	  mRenderStyle(renderStyle),
	  mInputManager(window),
	  mText(text) {
	if (mCharacterInputType == CharacterInputType::Custom) {
		for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
			KeyboardCommand command;
			command.key = key;
			command.normalMode = (char)('a' + (key - GLFW_KEY_A));
			command.shiftMode = (char)std::toupper(command.normalMode);
			command.altMode = '\0';
			mKeyboardCommands.push_back(command);
		}

		mKeyboardCommands.push_back({ GLFW_KEY_0, '0', '=', '}' });
		mKeyboardCommands.push_back({ GLFW_KEY_1, '1', '!', '\0' });
		mKeyboardCommands.push_back({ GLFW_KEY_2, '2', '"', '@' });
		mKeyboardCommands.push_back({ GLFW_KEY_3, '3', '#', '\0' });
		mKeyboardCommands.push_back({ GLFW_KEY_4, '4', '\0', '$' });
		mKeyboardCommands.push_back({ GLFW_KEY_5, '5', '%', '\0' });
		mKeyboardCommands.push_back({ GLFW_KEY_6, '6', '&', '\0' });
		mKeyboardCommands.push_back({ GLFW_KEY_7, '7', '/', '{' });
		mKeyboardCommands.push_back({ GLFW_KEY_8, '8', '(', '[' });
		mKeyboardCommands.push_back({ GLFW_KEY_MINUS, '+', '?', '\\' });

		mKeyboardCommands.push_back({ GLFW_KEY_COMMA, ',', ';', '\0' });
		mKeyboardCommands.push_back({ GLFW_KEY_PERIOD, '.', ':', '\0' });
		mKeyboardCommands.push_back({ GLFW_KEY_SPACE, ' ', '\0', '\0' });
		mKeyboardCommands.push_back({ GLFW_KEY_SLASH, '-', '_', '\0' });
	}

	mKeyboardCommands.push_back({ GLFW_KEY_TAB, '\t', '\0', '\0' });
}

const LineTokens& TextView::currentLine() const {
	return mFormattedText.getLine((std::size_t)mInputState.caretPositionY);
}

//std::size_t TextView::currentLineNumber() const {
//	return currentLine().number;
//}
//
//std::size_t TextView::currentLineLength() const {
//	return currentLine().length();
//}
//
//std::size_t TextView::numLines() {
//	return mFormattedText.numLines();
//}
//
//std::pair<std::size_t, std::int64_t> TextView::getLineAndOffset(int offsetX) const {
//	auto& line = currentLine();
//	std::size_t lineIndex = line.number;
//	auto offset = (std::int64_t)line.offsetFromTextLine + mInputState.caretPositionX + offsetX;
//	return std::make_pair(lineIndex, offset);
//}

std::size_t TextView::currentLineNumber() const {
	return (std::size_t)mInputState.caretPositionY;
}

std::size_t TextView::currentLineLength() const {
	return mText.getLine(currentLineNumber()).length();
}

std::size_t TextView::numLines() {
	return mText.numLines();
}

std::pair<std::size_t, std::int64_t> TextView::getLineAndOffset(int offsetX) const {
	std::size_t lineIndex = currentLineNumber();
	auto offset = mInputState.caretPositionX + offsetX;
	return std::make_pair(lineIndex, offset);
}

void TextView::moveCaretX(std::int64_t diff) {
	auto viewPort = getTextViewPort();
	const auto charWidth = mFont.getAdvanceX('A');
	mInputState.caretPositionX += diff;

	if (diff < 0) {
		if (mInputState.caretPositionX < 0L) {
			moveCaretY(-1);

			if (numLines() > 0) {
				mInputState.caretPositionX = (std::int64_t)currentLineLength();
			} else {
				mInputState.caretPositionX = 0;
			}
		}
	} else {
		if (numLines() > 0) {
			if ((std::size_t)mInputState.caretPositionX > currentLineLength()) {
				moveCaretY(1);
				mInputState.caretPositionX = 1;
			}
		}
	}

	auto caretScreenPositionX = -std::max(mInputState.caretPositionX + diff, 0L) * charWidth;
	if (caretScreenPositionX < mInputState.viewPosition.x - viewPort.width) {
		mInputState.viewPosition.x -= diff * charWidth;
	}

	if (caretScreenPositionX > mInputState.viewPosition.x) {
		mInputState.viewPosition.x -= diff * charWidth;
	}
}

void TextView::moveCaretY(std::int64_t diff) {
	auto viewPort = getTextViewPort();
	const auto lineHeight = mFont.lineHeight();

	mInputState.caretPositionY += diff;

	if (mInputState.caretPositionY >= (std::int64_t)numLines()) {
		mInputState.caretPositionY = (std::int64_t)numLines() - 1;
	}

	if (mInputState.caretPositionY < 0) {
		mInputState.caretPositionY = 0;
	}

	auto caretScreenPositionY = -std::max(mInputState.caretPositionY + diff, 0L) * lineHeight;
	if (caretScreenPositionY < mInputState.viewPosition.y - viewPort.height) {
		mInputState.viewPosition.y -= diff * lineHeight;
	}

	if (caretScreenPositionY > mInputState.viewPosition.y) {
		mInputState.viewPosition.y -= diff * lineHeight;
	}

	if (!(-mInputState.viewPosition.y + viewPort.height >= -caretScreenPositionY
		  && -mInputState.viewPosition.y <= -caretScreenPositionY)) {
		mInputState.viewPosition.y = -mInputState.caretPositionY * lineHeight + viewPort.height / 2.0f;
	}

	if (mInputState.viewPosition.y > 0) {
		mInputState.viewPosition.y = 0;
	}

	mInputState.caretPositionX = std::min((std::size_t)mInputState.caretPositionX, currentLineLength());
}

void TextView::updateViewMovement(const WindowState& windowState) {
	auto viewPort = getTextViewPort();
	const auto lineHeight = mFont.lineHeight();

	const auto pageMoveSpeed = (std::int64_t)std::round(viewPort.height / lineHeight);
	if (mInputManager.isKeyPressed(GLFW_KEY_PAGE_UP)) {
		mDrawCaret = true;
		mLastCaretUpdate = Helpers::timeNow();
		moveCaretY(-pageMoveSpeed);
	} else if (mInputManager.isKeyPressed(GLFW_KEY_PAGE_DOWN)) {
		mDrawCaret = true;
		mLastCaretUpdate = Helpers::timeNow();
		moveCaretY(pageMoveSpeed);
	}

	int caretPositionDiffY = 0;
	if (mInputManager.isKeyPressed(GLFW_KEY_UP)) {
		caretPositionDiffY = -1;
	} else if (mInputManager.isKeyPressed(GLFW_KEY_DOWN)) {
		caretPositionDiffY = 1;
	}

	if (caretPositionDiffY != 0) {
		mDrawCaret = true;
		mLastCaretUpdate = Helpers::timeNow();
		moveCaretY(caretPositionDiffY);
	}

	int caretPositionDiffX = 0;
	if (mInputManager.isKeyPressed(GLFW_KEY_LEFT)) {
		caretPositionDiffX = -1;
	} else if (mInputManager.isKeyPressed(GLFW_KEY_RIGHT)) {
		caretPositionDiffX = 1;
	}

	if (caretPositionDiffX != 0) {
		mDrawCaret = true;
		mLastCaretUpdate = Helpers::timeNow();
		moveCaretX(caretPositionDiffX);
	}

	if (windowState.hasScrolled()) {
		mInputState.viewPosition.y += mScrollSpeed * lineHeight * windowState.scrollY();

		if (mInputState.viewPosition.y > 0) {
			mInputState.viewPosition.y = 0;
		}
	}
}

void TextView::updateEditing(const WindowState& windowState) {
	auto setCaretX = [&](std::int64_t position) {
		mInputState.caretPositionX = position;
		mInputState.viewPosition.x = 0;
	};

	auto insertCharacter = [&](Char current) {
		auto lineAndOffset = getLineAndOffset();
		mText.insertAt(lineAndOffset.first, (std::size_t)lineAndOffset.second, current);
		updateFormattedText(getTextViewPort());
		moveCaretX(1);
	};

	auto deleteLine = [&](Text::DeleteLineMode mode) {
		if (mode == Text::DeleteLineMode::Start && mInputState.caretPositionY == 0) {
			return;
		}

		auto diff = mText.deleteLine(currentLineNumber(), mode);
		if (mode == Text::DeleteLineMode::Start) {
			mInputState.caretPositionX = diff.caretX;
		}

		updateFormattedText(getTextViewPort());

		if (mode == Text::DeleteLineMode::Start) {
			moveCaretY(-1);
		}
	};

	bool isShiftDown = mInputManager.isShiftDown();
	bool isAltDown = mInputManager.isAltDown();

	for (auto& command : mKeyboardCommands) {
		if (mInputManager.isKeyPressed(command.key)) {
			if (isShiftDown) {
				if (command.shiftMode != '\0') {
					insertCharacter(command.shiftMode);
				}
			} else if (isAltDown) {
				if (command.altMode != '\0') {
					insertCharacter(command.altMode);
				}
			} else {
				insertCharacter(command.normalMode);
			}
		}
	}

	if (mCharacterInputType == CharacterInputType::Native) {
		for (auto& codePoint : windowState.inputCharacters()) {
			insertCharacter(convertCodePointToChar(codePoint));
		}
	}

	if (mInputManager.isKeyPressed(GLFW_KEY_BACKSPACE)) {
		auto lineAndOffset = getLineAndOffset(-1);
		if (lineAndOffset.second >= 0) {
			mText.deleteAt(lineAndOffset.first, (std::size_t)lineAndOffset.second);
			updateFormattedText(getTextViewPort());
			moveCaretX(-1);
		} else {
			deleteLine(Text::DeleteLineMode::Start);
		}
	}

	if (mInputManager.isKeyPressed(GLFW_KEY_DELETE)) {
		auto lineAndOffset = getLineAndOffset();
		if (lineAndOffset.second < (std::int64_t)currentLineLength()) {
			mText.deleteAt(lineAndOffset.first, (std::size_t)lineAndOffset.second);
		} else {
			deleteLine(Text::DeleteLineMode::End);
		}
	}

	if (mInputManager.isKeyPressed(GLFW_KEY_ENTER)) {
		auto lineAndOffset = getLineAndOffset();
		mText.splitLine(lineAndOffset.first, (std::size_t)lineAndOffset.second);
		updateFormattedText(getTextViewPort());
		moveCaretY(1);
		setCaretX(0);
	}
}

void TextView::updateInput(const WindowState& windowState) {
	updateViewMovement(windowState);
	updateEditing(windowState);
	mInputManager.postUpdate();
}

void TextView::update(const WindowState& windowState) {
	auto timeNow = Helpers::timeNow();
	if (Helpers::durationMilliseconds(timeNow, mLastCaretUpdate) >= 500) {
		mDrawCaret = !mDrawCaret;
		mLastCaretUpdate = timeNow;
	}

	updateInput(windowState);
}

float TextView::getLineNumberSpacing() const {
	return (std::to_string(mText.numLines()).size() + 1) * mFont['A'].advanceX;
}

RenderViewPort TextView::getTextViewPort() {
	auto viewPort = mViewPort;
	viewPort.width -= mRenderStyle.sideSpacing * 2;
	viewPort.height -= mRenderStyle.bottomSpacing;
	viewPort.width -= getLineNumberSpacing();
	return viewPort;
}

namespace {
	void formattedBenchmark(const Font& font, FormatMode formatMode, const RenderStyle& renderStyle, const RenderViewPort& viewPort, const Text& text) {
		TextFormatter textFormatter(formatMode);
		for (int i = 0; i < 3; i++) {
			FormattedText formattedText;
			textFormatter.format(font, renderStyle, viewPort, text, formattedText);
		}

		int n = 10;
		auto t0 = Helpers::timeNow();
		for (int i = 0; i < n; i++) {
			FormattedText formattedText;
			textFormatter.format(font, renderStyle, viewPort, text, formattedText);
		}
		std::cout
			<< (Helpers::durationMicroseconds(Helpers::timeNow(), t0) / 1E3) / n << " ms"
			<< std::endl;
	}
}

void TextView::updateFormattedText(const RenderViewPort& viewPort) {
	if (mPerformFormattingType == PerformFormattingType::Full) {
		if (mLastViewPort.width != viewPort.width
			|| mLastViewPort.height != viewPort.height
			|| mLastViewPort.position != viewPort.position
			|| mText.hasChanged(mTextVersion)) {
			mFormattedText = {};
			mLastViewPort = viewPort;

			TextFormatter textFormatter(mFormatMode);
			auto t0 = Helpers::timeNow();
			textFormatter.format(mFont, mRenderStyle, viewPort, mText, mFormattedText);
			std::cout
				<< "Formatted text (lines = " << numLines() << ") in "
				<< (Helpers::durationMicroseconds(Helpers::timeNow(), t0) / 1E3) << " ms"
				<< std::endl;

			mInputState.caretPositionY = std::min(mInputState.caretPositionY, (std::int64_t)numLines() - 1);
		}
	}
}

PartialFormattedText TextView::performPartialFormatting(const RenderViewPort& viewPort, glm::vec2 position) {
	PartialFormattedText formattedText;
	formattedText.setNumLines(mText.numLines());

	TextFormatter textFormatter(mFormatMode);
	auto formatLine = [&](std::size_t lineIndex) {
		LineTokens lineTokens;
		textFormatter.formatLine(mFont, mRenderStyle, viewPort, mText.getLine(lineIndex), lineTokens);
		lineTokens.number = lineIndex;
		formattedText.addLine(lineIndex, lineTokens);
	};

	formatLine((std::size_t)mInputState.caretPositionY);

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

	return formattedText;
}

void TextView::render(TextRender& textRender) {
	auto viewPort = getTextViewPort();
	auto lineNumberSpacing = getLineNumberSpacing();

//	auto startTime = std::chrono::system_clock::now();
	auto drawPosition = glm::vec2(
		mInputState.viewPosition.x + mRenderStyle.sideSpacing,
		mInputState.viewPosition.y + mRenderStyle.topSpacing);

	updateFormattedText(viewPort);

	std::unique_ptr<BaseFormattedText> ownedFormattedText;
	BaseFormattedText* formattedText = nullptr;
	switch (mPerformFormattingType) {
		case PerformFormattingType::Full:
			formattedText = &mFormattedText;
			break;
		case PerformFormattingType::Partial:
			ownedFormattedText = std::make_unique<PartialFormattedText>(performPartialFormatting(viewPort, drawPosition + glm::vec2(lineNumberSpacing, 0.0f)));
			formattedText = ownedFormattedText.get();
			break;
	}

//	textRender.renderLineNumbers(
//		mFont,
//		mRenderStyle,
//		viewPort,
//		formattedText,
//		drawPosition);
//
//	textRender.render(
//		mFont,
//		mRenderStyle,
//		viewPort,
//		formattedText,
//		drawPosition + glm::vec2(lineNumberSpacing, 0.0f));

	//
	textRender.render(
		mFont,
		mRenderStyle,
		viewPort,
		*formattedText,
		drawPosition,
		lineNumberSpacing);

	if (mDrawCaret) {
		textRender.renderCaret(
			mFont,
			mRenderStyle,
			viewPort,
			*formattedText,
			{ lineNumberSpacing + mRenderStyle.sideSpacing, mRenderStyle.topSpacing },
			mInputState);
	}

//	std::cout
//		<< "Render time: " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - startTime).count() / 1E3)
//		<< "ms"
//		<< std::endl;
}
