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
	auto createInsertCharacterCommand = [&](int key, Char normalMode, Char shiftMode, Char altMode) {
		KeyboardCommand command;
		command.key = key;

		command.normalMode = [&, normalMode]() {
			insertAction(normalMode);
		};

		if (shiftMode != '\0') {
			command.shiftMode = [&, shiftMode]() {
				insertAction(shiftMode);
			};
		}

		if (shiftMode != '\0') {
			command.altMode = [&, altMode]() {
				insertAction(altMode);
			};
		}
		
		mKeyboardCommands.push_back(std::move(command));
	};

	if (mCharacterInputType == CharacterInputType::Custom) {
		for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
			auto character = (Char)('a' + (key - GLFW_KEY_A));
			createInsertCharacterCommand(key, character, (Char)std::toupper(character), '\0');
		}

		createInsertCharacterCommand(GLFW_KEY_0, '0', '=', '}');
		createInsertCharacterCommand(GLFW_KEY_1, '1', '!', '\0');
		createInsertCharacterCommand(GLFW_KEY_2, '2', '"', '@');
		createInsertCharacterCommand(GLFW_KEY_3, '3', '#', '\0');
		createInsertCharacterCommand(GLFW_KEY_4, '4', '\0', '$');
		createInsertCharacterCommand(GLFW_KEY_5, '5', '%', '\0');
		createInsertCharacterCommand(GLFW_KEY_6, '6', '&', '\0');
		createInsertCharacterCommand(GLFW_KEY_7, '7', '/', '{');
		createInsertCharacterCommand(GLFW_KEY_8, '8', '(', '[');
		createInsertCharacterCommand(GLFW_KEY_MINUS, '+', '?', '\\');

		createInsertCharacterCommand(GLFW_KEY_COMMA, ',', ';', '\0');
		createInsertCharacterCommand(GLFW_KEY_PERIOD, '.', ':', '\0');
		createInsertCharacterCommand(GLFW_KEY_SPACE, ' ', '\0', '\0');
		createInsertCharacterCommand(GLFW_KEY_SLASH, '-', '_', '\0');
	}

	createInsertCharacterCommand(GLFW_KEY_TAB, '\t', '\0', '\0');

	mKeyboardCommands.emplace_back(KeyboardCommand { GLFW_KEY_BACKSPACE, [&]() { backspaceAction(); }, {}, {}, {} });
	mKeyboardCommands.emplace_back(KeyboardCommand { GLFW_KEY_DELETE, [&]() { deleteAction(); }, {}, {}, {} });
	mKeyboardCommands.emplace_back(KeyboardCommand { GLFW_KEY_ENTER, [&]() { insertLine(); }, {}, {}, {} });
	mKeyboardCommands.emplace_back(KeyboardCommand { GLFW_KEY_V, {}, [&]() { paste(); }, {}, {} });
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
	mShowSelection = false;

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

void TextView::setCaretX(std::int64_t position) {
	mViewMoved = true;
	mInputState.caretPositionX = position;
	mInputState.viewPosition.x = 0;
}

void TextView::clampViewPositionY(float caretScreenPositionY) {
	const auto viewPort = getTextViewPort();
	const auto lineHeight = mFont.lineHeight();

	if (!(-mInputState.viewPosition.y + viewPort.height >= -caretScreenPositionY
		  && -mInputState.viewPosition.y <= -caretScreenPositionY)) {
		mInputState.viewPosition.y = -mInputState.caretPositionY * lineHeight + viewPort.height / 2.0f;
	}

	if (mInputState.viewPosition.y > 0) {
		mInputState.viewPosition.y = 0;
	}

	auto maxViewHeight = std::ceil((numLines() * mFont.lineHeight() - viewPort.height) / mFont.lineHeight()) * mFont.lineHeight();
	if (mInputState.viewPosition.y < -maxViewHeight) {
		mInputState.viewPosition.y = -maxViewHeight;
	}

	auto contentHeight = numLines() * lineHeight;
	if (contentHeight < viewPort.height) {
		mInputState.viewPosition.y = 0;
	}
}

void TextView::moveCaretY(std::int64_t diff) {
	mShowSelection = false;
	mViewMoved = true;

	auto viewPort = getTextViewPort();
	const auto lineHeight = mFont.lineHeight();

	mInputState.caretPositionY += diff;

	if (mInputState.caretPositionY >= (std::int64_t)numLines()) {
		mInputState.caretPositionY = (std::int64_t)numLines() - 1;
		diff = 0;
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

	clampViewPositionY(caretScreenPositionY);
	mInputState.caretPositionX = std::min((std::size_t)mInputState.caretPositionX, currentLineLength());
}

void TextView::moveViewY(float diff) {
	auto viewPort = getTextViewPort();
	mViewMoved = true;
	mInputState.viewPosition.y += diff;

	if (mInputState.viewPosition.y > 0) {
		mInputState.viewPosition.y = 0;
	}

	auto maxViewHeight = std::ceil((numLines() * mFont.lineHeight() - viewPort.height) / mFont.lineHeight()) * mFont.lineHeight();
	if (mInputState.viewPosition.y < -maxViewHeight) {
		mInputState.viewPosition.y = -maxViewHeight;
	}

	auto contentHeight = numLines() * mFont.lineHeight();
	if (contentHeight < viewPort.height) {
		mInputState.viewPosition.y = 0;
	}
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

	if (mInputManager.isKeyPressed(GLFW_KEY_HOME)) {
		mDrawCaret = true;
		mLastCaretUpdate = Helpers::timeNow();
		moveCaretY(-mInputState.caretPositionY);
	} else if (mInputManager.isKeyPressed(GLFW_KEY_END)) {
		mDrawCaret = true;
		mLastCaretUpdate = Helpers::timeNow();
		moveCaretY((std::int64_t)numLines() - mInputState.caretPositionY);
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
		moveViewY((float)(mScrollSpeed * lineHeight * windowState.scrollY()));
	}
}

void TextView::insertCharacter(Char character) {
	auto lineAndOffset = getLineAndOffset();
	mText.insertAt(lineAndOffset.first, (std::size_t)lineAndOffset.second, character);
	updateFormattedText(getTextViewPort());
	moveCaretX(1);
}

void TextView::insertAction(Char character) {
	if (!mInputState.selection.isSingle() && mShowSelection) {
		replaceSelection(character);
	} else {
		insertCharacter(character);
	}
}

void TextView::insertLine() {
	auto lineAndOffset = getLineAndOffset();
	mText.splitLine(lineAndOffset.first, (std::size_t)lineAndOffset.second);
	updateFormattedText(getTextViewPort());
	moveCaretY(1);
	setCaretX(0);
}

void TextView::paste() {
	if (const char* text = glfwGetClipboardString(mWindow)) {
		auto lineAndOffset = getLineAndOffset();

		auto stringText = Helpers::fromString<String>(text);
		auto charIndex = (std::size_t)lineAndOffset.second;
		auto numInserted = 0;
		for (auto& character : stringText) {
			mText.insertAt(lineAndOffset.first, charIndex, character);
			charIndex++;
			numInserted++;
		}

		updateFormattedText(getTextViewPort());
		moveCaretX(numInserted);
	}
}

void TextView::deleteLine(Text::DeleteLineMode mode) {
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
}

void TextView::deleteSelection() {
	mText.deleteSelection(mInputState.selection);
	mInputState.caretPositionX = (std::size_t)mInputState.selection.startX;
	mInputState.caretPositionY = (std::size_t)mInputState.selection.startY;
	mInputState.selection.setSingle((std::size_t)mInputState.caretPositionX, (std::size_t)mInputState.caretPositionY);

	mShowSelection = false;

	auto caretScreenPositionY = -mInputState.caretPositionY * mFont.lineHeight();
	clampViewPositionY(caretScreenPositionY);

	mViewMoved = true;
	updateFormattedText(getTextViewPort());
}

void TextView::backspaceAction() {
	if (mInputState.selection.isSingle() || !mShowSelection) {
		auto lineAndOffset = getLineAndOffset(-1);
		if (lineAndOffset.second >= 0) {
			mText.deleteAt(lineAndOffset.first, (std::size_t)lineAndOffset.second);
			updateFormattedText(getTextViewPort());
			moveCaretX(-1);
		} else {
			deleteLine(Text::DeleteLineMode::Start);
		}
	} else {
		deleteSelection();
	}
}

void TextView::deleteAction() {
	if (mInputState.selection.isSingle() || !mShowSelection) {
		auto lineAndOffset = getLineAndOffset();
		if (lineAndOffset.second < (std::int64_t)currentLineLength()) {
			mText.deleteAt(lineAndOffset.first, (std::size_t)lineAndOffset.second);
		} else {
			deleteLine(Text::DeleteLineMode::End);
		}
	} else {
		deleteSelection();
	}
}

void TextView::replaceSelection(Char character) {
	deleteSelection();
	insertCharacter(character);
}

void TextView::updateEditing(const WindowState& windowState) {
	bool isControlDown = mInputManager.isControlDown();
	bool isShiftDown = mInputManager.isShiftDown();
	bool isAltDown = mInputManager.isAltDown();

	for (auto& command : mKeyboardCommands) {
		if (mInputManager.isKeyPressed(command.key)) {
			if (isControlDown) {
				if (command.controlMode) {
					command.controlMode();
				}
			} else if (isShiftDown) {
				if (command.shiftMode) {
					command.shiftMode();
				}
			} else if (isAltDown) {
				if (command.altMode) {
					command.altMode();
				}
			} else {
				command.normalMode();
			}
		}
	}

	if (mCharacterInputType == CharacterInputType::Native) {
		for (auto& codePoint : windowState.inputCharacters()) {
			insertAction(convertCodePointToChar(codePoint));
		}
	}
}

std::pair<std::int64_t, std::int64_t> TextView::getMouseTextPosition() {
	double mouseX;
	double mouseY;
	glfwGetCursorPos(mWindow, &mouseX, &mouseY);

	auto drawPosition = getDrawPosition();
	auto textY = (std::int64_t)std::floor((-drawPosition.y + mouseY) / mFont.lineHeight());
	auto relativeMousePositionX = mouseX - getLineNumberSpacing() - drawPosition.x;

	if (textY < 0) {
		textY = 0;
	}

	if (textY >= (std::int64_t)numLines()) {
		textY = numLines() - 1;
	}

	if (mPerformFormattingType == PerformFormattingType::Partial) {
		auto* formattedText = (PartialFormattedText*)mFormattedText.get();
		if (!formattedText->hasLine((std::size_t)textY)) {
			TextFormatter textFormatter(mFormatMode);
			formatLinePartialMode(getTextViewPort(), textFormatter, *formattedText, (std::size_t)textY);
		}
	}

	auto textX = (std::int64_t)mTextMetrics.getCharIndexFromScreenPosition(
		*mFormattedText,
		mFormattedText->getLine((std::size_t)textY).number,
		(float)relativeMousePositionX);

	return std::make_pair(textX, textY);
}

void TextView::updateTextSelection(const WindowState& windowState) {
	if (glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		auto mouseTextPosition = getMouseTextPosition();

		if (mSelectionStarted) {
			mPotentialSelection.endX = (std::size_t)mouseTextPosition.first;
			mPotentialSelection.endY = (std::size_t)mouseTextPosition.second;

			double mouseX;
			double mouseY;
			glfwGetCursorPos(mWindow, &mouseX, &mouseY);

			if (mouseY <= 0) {
				moveViewY((float)(-mouseY));
				updateFormattedText(getTextViewPort());
			}

			if (mouseY >= windowState.height()) {
				moveViewY((float)(windowState.height() - mouseY));
				updateFormattedText(getTextViewPort());
			}

			mInputState.selection = mPotentialSelection;
			if (mInputState.selection.endY < mInputState.selection.startY) {
				std::swap(mInputState.selection.startX, mInputState.selection.endX);
				std::swap(mInputState.selection.startY, mInputState.selection.endY);
			}

			if (mPerformFormattingType == PerformFormattingType::Partial) {
				auto* formattedText = (PartialFormattedText*)mFormattedText.get();
				auto viewPort = getTextViewPort();
				TextFormatter textFormatter(mFormatMode);

//				for (std::size_t lineIndex = mInputState.selection.startY; lineIndex <= mInputState.selection.endY; lineIndex++) {
//					if (!formattedText->hasLine((std::size_t)lineIndex)) {
//						formatLinePartialMode(viewPort, textFormatter, *formattedText, lineIndex);
//					}
//				}

				if (!formattedText->hasLine((std::size_t)mInputState.selection.startY)) {
					formatLinePartialMode(viewPort, textFormatter, *formattedText, (std::size_t)mInputState.selection.startY);
				}

				if (!formattedText->hasLine((std::size_t)mInputState.selection.endY)) {
					formatLinePartialMode(viewPort, textFormatter, *formattedText, (std::size_t)mInputState.selection.endY);
				}
			}
		} else {
			mPotentialSelection.startX = (std::size_t)mouseTextPosition.first;
			mPotentialSelection.startY = (std::size_t)mouseTextPosition.second;
		}
	}

	if (mInputManager.isMouseDragMove(GLFW_MOUSE_BUTTON_LEFT)) {
		if (!mSelectionStarted) {
			mSelectionStarted = true;
			mShowSelection = true;
			mViewMoved = true;
//			std::cout << "selection started" << std::endl;
		}
	} else if (mSelectionStarted) {
		mSelectionStarted = false;

//		std::cout
//			<< "selection ended: "
//	  		<< mInputState.selection.startX << ", " << mInputState.selection.startY
//			<< " -> "
//	  		<< mInputState.selection.endX << ", " << mInputState.selection.endY
//			<< std::endl;
	}
}

void TextView::updateMouseMovement(const WindowState& windowState) {
	if (windowState.isLeftMouseButtonPressed()) {
		updateFormattedText(getTextViewPort());

		auto mouseTextPosition = getMouseTextPosition();
		mInputState.caretPositionX = mouseTextPosition.first;
		mInputState.caretPositionY = mouseTextPosition.second;

		if (!mSelectionStarted) {
			mShowSelection = false;
		}

//		std::cout << "clicked" << std::endl;
	}

	updateTextSelection(windowState);
}

void TextView::updateInput(const WindowState& windowState) {
	updateViewMovement(windowState);
	updateEditing(windowState);
	updateMouseMovement(windowState);
	mInputManager.postUpdate();
}

Text& TextView::text() {
	return mText;
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

void TextView::updateFormattedText() {
	updateFormattedText(getTextViewPort());
}

void TextView::formatLinePartialMode(const RenderViewPort& viewPort,
									 TextFormatter& textFormatter,
									 PartialFormattedText& formattedText,
									 std::size_t lineIndex) {
	LineTokens lineTokens;
	textFormatter.formatLine(mFont, mRenderStyle, viewPort, mText.getLine(lineIndex), lineTokens);
	lineTokens.number = lineIndex;
	formattedText.addLine(lineIndex, lineTokens);
}

PartialFormattedText TextView::performPartialFormatting(const RenderViewPort& viewPort, glm::vec2 position) {
	PartialFormattedText formattedText;
	formattedText.setNumLines(mText.numLines());

	TextFormatter textFormatter(mFormatMode);
	auto formatLine = [&](std::size_t lineIndex) {
		formatLinePartialMode(viewPort, textFormatter, formattedText, lineIndex);
	};

	if ((std::size_t)mInputState.caretPositionY < mText.numLines()) {
		formatLine((std::size_t)mInputState.caretPositionY);
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

	if (mInputState.selection.startY != mInputState.selection.endY && mShowSelection) {
//		for (std::size_t lineIndex = mInputState.selection.startY; lineIndex <= std::min(mInputState.selection.endY, numLines() - 1); lineIndex++) {
//			formatLine(lineIndex);
//		}

		formatLine(mInputState.selection.startY);
		formatLine(std::min(mInputState.selection.endY, numLines() - 1));
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

	textRender.renderLineNumbers(
		mFont,
		mRenderStyle,
		viewPort,
		*formattedText,
		drawPosition);

	textRender.render(
		mFont,
		mRenderStyle,
		viewPort,
		*formattedText,
		drawPosition + glm::vec2(lineNumberSpacing, 0.0f),
		0.0f);

	//
	// textRender.render(
	// 	mFont,
	// 	mRenderStyle,
	// 	viewPort,
	// 	*formattedText,
	// 	drawPosition,
	// 	lineNumberSpacing);

	if (mShowSelection) {
		mTextSelectionRender.render(
			windowState,
			mFont,
			viewPort,
			mTextMetrics,
			*formattedText,
			{
				drawPosition.x + lineNumberSpacing,
				drawPosition.y
			},
			mInputState);
	} else {
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
	}

//	std::cout
//		<< "Render time: " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - startTime).count() / 1E3)
//		<< "ms"
//		<< std::endl;
}
