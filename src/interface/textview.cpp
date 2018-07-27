#include "textview.h"
#include "../rendering/textrender.h"
#include "../rendering/renderstyle.h"
#include "../rendering/renderviewport.h"
#include "../rendering/font.h"

#include "../text/text.h"
#include "inputmanager.h"
#include "../windowstate.h"
#include "../rendering/glhelpers.h"
#include "../rendering/shadercompiler.h"

#include <chrono>
#include <iostream>
#include <algorithm>
#include <memory>
#include <codecvt>
#include <locale>
#include <glm/gtc/matrix_transform.hpp>

namespace {
	std::string print(char16_t current) {
		std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> cv;
		return cv.to_bytes(std::u16string { current });
	}

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
	  mRenderStyle(renderStyle),
	  mTextMetrics(mFont, mRenderStyle),
	  mViewPort(viewPort),
	  mInputManager(window),
	  mText(text) {
	if (mCharacterInputType == CharacterInputType::Custom) {
		for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
			KeyboardCommand command;
			command.key = key;
			command.normalMode = (Char)('a' + (key - GLFW_KEY_A));
			command.shiftMode = (Char)std::toupper(command.normalMode);
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
	auto lineIndex = (std::size_t)mInputState.caretPositionY;
	return mFormattedText->getLine(lineIndex);
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

float TextView::currentLineWidth() const {
	return mTextMetrics.getLineWidth(currentLine(), 0, nullptr);
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

			auto lineWidth = currentLineWidth();
			if (lineWidth >= viewPort.width) {
				mInputState.viewPosition.x = -(lineWidth - viewPort.width);
			} else {
				mInputState.viewPosition.x = 0.0f;
			}

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
				mInputState.caretPositionX = 0;
				mInputState.viewPosition.x = 0.0f;
			}
		}
	}

	auto caretScreenPositionX = -std::max(mInputState.caretPositionX + diff, 0L) * charWidth;
	if (caretScreenPositionX <= mInputState.viewPosition.x - viewPort.width + charWidth) {
		mInputState.viewPosition.x -= diff * charWidth;
	}

	if (caretScreenPositionX > mInputState.viewPosition.x) {
		mInputState.viewPosition.x -= diff * charWidth;
	}
}

void TextView::moveCaretY(std::int64_t diff) {
	mViewMoved = true;

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
		mViewMoved = true;
		mInputState.viewPosition.y += mScrollSpeed * lineHeight * windowState.scrollY();

		if (mInputState.viewPosition.y > 0) {
			mInputState.viewPosition.y = 0;
		}

		auto maxViewHeight = std::ceil((numLines() * mFont.lineHeight() - viewPort.height) / mFont.lineHeight()) * mFont.lineHeight();
		if (mInputState.viewPosition.y < -maxViewHeight) {
			mInputState.viewPosition.y = -maxViewHeight;
		}
	}
}

void TextView::updateEditing(const WindowState& windowState) {
	auto setCaretX = [&](std::int64_t position) {
		mViewMoved = true;
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

std::size_t TextView::getCharIndexFromScreenPosition(std::size_t lineIndex, float screenPositionX) const {
	float currentCharOffset = 0.0f;
	std::size_t currentCharIndex = 0;
	for (auto& token : mFormattedText->getLine(lineIndex).tokens) {
		for (auto character : token.text) {
			auto advanceX = mRenderStyle.getAdvanceX(mFont, character);

			if (screenPositionX >= currentCharOffset && screenPositionX <= currentCharOffset + advanceX) {
				break;
			}

			currentCharOffset += advanceX;
			currentCharIndex++;
		}
	}

	return currentCharIndex;
}

void TextView::updateMouseMovement(const WindowState& windowState) {
	auto getMouseTextPosition = [&]() {
		double mouseX;
		double mouseY;
		glfwGetCursorPos(mWindow, &mouseX, &mouseY);

		auto drawPosition = glm::vec2(
			mInputState.viewPosition.x + mRenderStyle.sideSpacing,
			mInputState.viewPosition.y + mRenderStyle.topSpacing);

		auto textY = (std::int64_t)std::floor((-drawPosition.y + mouseY) / mFont.lineHeight());
		auto relativeMousePositionX = mouseX - getLineNumberSpacing() - drawPosition.x;

		if (mPerformFormattingType == PerformFormattingType::Partial) {
			auto* formattedText = (PartialFormattedText*)mFormattedText.get();
			if (!formattedText->hasLine((std::size_t)textY)) {
				TextFormatter textFormatter(mFormatMode);
				LineTokens lineTokens;
				textFormatter.formatLine(mFont, mRenderStyle, getTextViewPort(), mText.getLine((std::size_t)textY), lineTokens);
				lineTokens.number = (std::size_t)textY;
				formattedText->addLine((std::size_t)textY, lineTokens);
			}
		}

		auto textX = (std::int64_t)getCharIndexFromScreenPosition(mFormattedText->getLine((std::size_t)textY).number, (float)relativeMousePositionX);

		return std::make_pair(textX, textY);
	};

	if (windowState.isLeftMouseButtonPressed()) {
		updateFormattedText(getTextViewPort());

		auto mouseTextPosition = getMouseTextPosition();
		mInputState.caretPositionX = mouseTextPosition.first;
		mInputState.caretPositionY = mouseTextPosition.second;
//		mViewMoved = true;
	}

	if (glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		auto mouseTextPosition = getMouseTextPosition();

		if (!mSelectionStarted) {
			mPotentialSelectionStartX = mouseTextPosition.first;
			mPotentialSelectionStartY = mouseTextPosition.second;
		} else {
			mPotentialSelectionEndX = mouseTextPosition.first;
			mPotentialSelectionEndY = mouseTextPosition.second;

			mInputState.selectionStartX = mPotentialSelectionStartX;
			mInputState.selectionStartY = mPotentialSelectionStartY;
			mInputState.selectionEndX = mPotentialSelectionEndX;
			mInputState.selectionEndY = mPotentialSelectionEndY;

			if (mInputState.selectionEndY < mInputState.selectionStartY) {
				std::swap(mInputState.selectionStartX, mInputState.selectionEndX);
				std::swap(mInputState.selectionStartY, mInputState.selectionEndY);
			}

			if (mPerformFormattingType == PerformFormattingType::Partial) {
				auto* formattedText = (PartialFormattedText*)mFormattedText.get();

				bool needUpdate = false;
				for (std::int64_t lineIndex = mInputState.selectionStartY; lineIndex <= mInputState.selectionEndY; lineIndex++) {
					needUpdate |= !formattedText->hasLine((std::size_t)lineIndex);
					if (needUpdate) {
						break;
					}
				}

				if (needUpdate) {
					mViewMoved = true;
					updateFormattedText(getTextViewPort());
				}
			}
		}
	}

	if (mInputManager.isMouseButtonHoldDown(GLFW_MOUSE_BUTTON_LEFT, 10)) {
		if (!mSelectionStarted) {
			mSelectionStarted = true;
			std::cout << "selection started" << std::endl;
		}
	} else if (mSelectionStarted) {
		mSelectionStarted = false;

		std::cout
			<< "selection ended: "
	  		<< mInputState.selectionStartX << ", " << mInputState.selectionStartY
			<< " -> "
	  		<< mInputState.selectionEndX << ", " << mInputState.selectionEndY
			<< std::endl;
	}
}

void TextView::updateInput(const WindowState& windowState) {
	updateViewMovement(windowState);
	updateEditing(windowState);
	updateMouseMovement(windowState);
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

RenderViewPort TextView::getTextViewPort() const {
	auto viewPort = mViewPort;
	viewPort.width -= mRenderStyle.sideSpacing * 2;
	viewPort.height -= mRenderStyle.bottomSpacing;
	viewPort.width -= getLineNumberSpacing();
	return viewPort;
}

glm::vec2 TextView::getDrawPosition() const {
	return glm::vec2(
		mInputState.viewPosition.x + mRenderStyle.sideSpacing,
		mInputState.viewPosition.y + mRenderStyle.topSpacing);
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
				TextFormatter textFormatter(mFormatMode);
				auto t0 = Helpers::timeNow();
				auto formattedText = std::make_unique<FormattedText>();
				textFormatter.format(mFont, mRenderStyle, viewPort, mText, *formattedText);
				mFormattedText = std::move(formattedText);
				std::cout
					<< "Formatted text (lines = " << numLines() << ") in "
					<< (Helpers::durationMicroseconds(Helpers::timeNow(), t0) / 1E3) << " ms"
					<< std::endl;

				mInputState.caretPositionY = std::min(mInputState.caretPositionY, (std::int64_t) numLines() - 1);
				break;
			}
			case PerformFormattingType::Partial: {
				auto t0 = Helpers::timeNow();
				mViewMoved = false;
				mFormattedText = std::make_unique<PartialFormattedText>(
				performPartialFormatting(viewPort, getDrawPosition() + glm::vec2(getLineNumberSpacing(), 0.0f)));

				std::cout
					<< "Partial formatted text (lines = " << numLines() << ") in "
					<< (Helpers::durationMicroseconds(Helpers::timeNow(), t0) / 1E3) << " ms"
					<< std::endl;
				break;
			}
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

	if (mInputState.selectionStartY != mInputState.selectionEndY) {
		for (std::int64_t lineIndex = mInputState.selectionStartY; lineIndex <= mInputState.selectionEndY; lineIndex++) {
			formatLine((std::size_t)lineIndex);
		}
	}

	return formattedText;
}

void TextView::render(const WindowState& windowState, TextRender& textRender) {
	auto viewPort = getTextViewPort();
	auto lineNumberSpacing = getLineNumberSpacing();

//	auto startTime = std::chrono::system_clock::now();
	auto drawPosition = getDrawPosition();
	updateFormattedText(viewPort);
	auto formattedText = mFormattedText.get();

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
			mTextMetrics,
			viewPort,
			*formattedText,
			{ lineNumberSpacing + mRenderStyle.sideSpacing, mRenderStyle.topSpacing },
			mInputState);
	}

	mTextSelectionRender.render(
		windowState,
		mFont,
		mTextMetrics,
		*formattedText,
		{
			mInputState.viewPosition.x + lineNumberSpacing + mRenderStyle.sideSpacing,
			mInputState.viewPosition.y + mRenderStyle.topSpacing
		},
		mInputState);

//	std::cout
//		<< "Render time: " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - startTime).count() / 1E3)
//		<< "ms"
//		<< std::endl;
}
