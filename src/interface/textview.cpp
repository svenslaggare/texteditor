#include "textview.h"
#include "../rendering/textrender.h"
#include "../rendering/renderstyle.h"
#include "../rendering/renderviewport.h"
#include "../rendering/font.h"

#include "../text/text.h"
#include "inputmanager.h"
#include "../windowstate.h"
#include "../rendering/common/glhelpers.h"
#include "../rendering/common/shadercompiler.h"
#include "../text/incrementalformattedtext.h"

#include <chrono>
#include <algorithm>
#include <memory>
#include <codecvt>
#include <locale>

namespace {
	std::string print(char16_t current) {
		std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> cv;
		return cv.to_bytes(std::u16string { current });
	}

	Char convertCodePointToChar(CodePoint codePoint) {
		return (Char)codePoint;
	}
}

glm::vec2 InputState::getDrawPosition(const RenderStyle& renderStyle) const {
	return glm::vec2(
		viewPosition.x + renderStyle.sideSpacing,
		viewPosition.y + renderStyle.topSpacing);
}

TextView::TextView(GLFWwindow* window,
				   Font& font,
				   std::unique_ptr<FormatterRules> rules,
				   const RenderViewPort& viewPort,
				   const RenderStyle& renderStyle,
				   Text& text)
	: mWindow(window),
	  mFont(font),
	  mRenderStyle(renderStyle),
	  mTextMetrics(mFont, mRenderStyle),
	  mViewPort(viewPort),
	  mTextOperations(
		PerformFormattingType::Incremental,
	  	font,
	  	std::move(rules),
	  	renderStyle,
	  	text,
	  	mInputState),
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

const FormattedLine& TextView::currentLine() const {
	auto lineIndex = (std::size_t)mInputState.caretLineIndex;
	return mTextOperations.formattedText()->getLine(lineIndex);
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
	return (std::size_t)mInputState.caretLineIndex;
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

void TextView::moveCaretX(std::int64_t diff) {
	std::int64_t diffSign = 0;
	if (diff > 0) {
		diffSign = 1;
	} else if (diff < 0) {
		diffSign = -1;
	}

	mInputState.showSelection = false;

	auto viewPort = getTextViewPort();
	const auto charWidth = mFont.getAdvanceX('A');
	mInputState.caretCharIndex += diff;

	if (diff < 0) {
		if (mInputState.caretCharIndex < 0L) {
			moveCaretY(-1);

			auto lineWidth = currentLineWidth();
			if (lineWidth >= viewPort.width) {
				mInputState.viewPosition.x = -(lineWidth - viewPort.width);
			} else {
				mInputState.viewPosition.x = 0.0f;
			}

			if (numLines() > 0) {
				mInputState.caretCharIndex = (std::int64_t)currentLineLength();
				diff = 2;
			} else {
				mInputState.caretCharIndex = 0;
			}
		}
	} else {
		if (numLines() > 0) {
			if ((std::size_t)mInputState.caretCharIndex > currentLineLength()) {
				moveCaretY(1);
				mInputState.caretCharIndex = 0;
				mInputState.viewPosition.x = 0.0f;
			}
		}
	}

	auto caretScreenPositionX = -std::max(mInputState.caretCharIndex + diffSign, 0L) * charWidth;
	if (caretScreenPositionX <= mInputState.viewPosition.x - viewPort.width + charWidth) {
		mInputState.viewPosition.x -= diff * charWidth;
	}

	if (caretScreenPositionX > mInputState.viewPosition.x) {
		mInputState.viewPosition.x -= diff * charWidth;
	}

	if (mInputState.viewPosition.x > 0) {
		mInputState.viewPosition.x = 0;
	}

	auto maxViewPositionX = std::max(currentLineWidth() - viewPort.width, 0.0f);
	if (mInputState.viewPosition.x < -maxViewPositionX) {
		mInputState.viewPosition.x = -maxViewPositionX;
	}
}

void TextView::setCaretX(std::int64_t position) {
	mTextOperations.viewMoved();
	mInputState.caretCharIndex = position;
	mInputState.viewPosition.x = 0;
}

void TextView::clampViewPositionY(float caretScreenPositionY) {
	const auto viewPort = getTextViewPort();
	const auto lineHeight = mFont.lineHeight();

	if (!(-mInputState.viewPosition.y + viewPort.height >= -caretScreenPositionY
		  && -mInputState.viewPosition.y <= -caretScreenPositionY)) {
		mInputState.viewPosition.y = -mInputState.caretLineIndex * lineHeight + viewPort.height / 2.0f;
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
	mInputState.showSelection = false;
	mTextOperations.viewMoved();

	auto viewPort = getTextViewPort();
	const auto lineHeight = mFont.lineHeight();

	mInputState.caretLineIndex += diff;

	if (mInputState.caretLineIndex >= (std::int64_t)numLines()) {
		mInputState.caretLineIndex = (std::int64_t)numLines() - 1;
		diff = 0;
	}

	if (mInputState.caretLineIndex < 0) {
		mInputState.caretLineIndex = 0;
	}

	auto caretScreenPositionY = -std::max(mInputState.caretLineIndex + diff, 0L) * lineHeight;
	if (caretScreenPositionY < mInputState.viewPosition.y - viewPort.height) {
		mInputState.viewPosition.y -= diff * lineHeight;
	}

	if (caretScreenPositionY > mInputState.viewPosition.y) {
		mInputState.viewPosition.y -= diff * lineHeight;
	}

	clampViewPositionY(caretScreenPositionY);
	mInputState.caretCharIndex = std::min((std::size_t)mInputState.caretCharIndex, currentLineLength());

	if (currentLineWidth() > -viewPort.width) {
		mInputState.viewPosition.x = 0;
	}
}

void TextView::moveViewY(float diff) {
	auto viewPort = getTextViewPort();
	mTextOperations.viewMoved();
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
		moveCaretY(-mInputState.caretLineIndex);
	} else if (mInputManager.isKeyPressed(GLFW_KEY_END)) {
		mDrawCaret = true;
		mLastCaretUpdate = Helpers::timeNow();
		moveCaretY((std::int64_t)numLines() - mInputState.caretLineIndex);
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
	mTextOperations.insertCharacter(getTextViewPort(), character);
	moveCaretX(1);
}

void TextView::insertAction(Char character) {
	if (!mInputState.selection.isSingle() && mInputState.showSelection) {
		replaceSelection(character);
	} else {
		insertCharacter(character);
	}
}

void TextView::insertLine() {
	mTextOperations.insertLine(getTextViewPort());
	moveCaretY(1);
	setCaretX(0);
}

void TextView::paste() {
	if (const char* rawPasteText = glfwGetClipboardString(mWindow)) {
		auto stringPasteText = Helpers::fromString<String>(rawPasteText);
		auto diffCaret = mTextOperations.paste(getTextViewPort(), stringPasteText);

		if (diffCaret.second > 0) {
			moveCaretY(diffCaret.second);
		}

		moveCaretX(diffCaret.first);
	}
}

void TextView::deleteLine(Text::DeleteLineMode mode) {
	if (mode == Text::DeleteLineMode::Start && mInputState.caretLineIndex == 0) {
		return;
	}

	mTextOperations.deleteLine(getTextViewPort(), mode);

	if (mode == Text::DeleteLineMode::Start) {
		moveCaretY(-1);
	}
}

void TextView::deleteSelection() {
	auto textSelection = mInputState.selection;
	mTextOperations.viewMoved();
	mTextOperations.deleteSelection(getTextViewPort(), textSelection);

	mInputState.caretCharIndex = (std::size_t)mInputState.selection.startX;
	mInputState.caretLineIndex = (std::size_t)mInputState.selection.startY;
	mInputState.selection.setSingle((std::size_t)mInputState.caretCharIndex, (std::size_t)mInputState.caretLineIndex);

	mInputState.showSelection = false;

	auto caretScreenPositionY = -mInputState.caretLineIndex * mFont.lineHeight();
	clampViewPositionY(caretScreenPositionY);
}

void TextView::deleteCharacter(std::int64_t charIndex) {
	mTextOperations.deleteCharacter(getTextViewPort(), (std::size_t)charIndex);
}

void TextView::backspaceAction() {
	if (mInputState.selection.isSingle() || !mInputState.showSelection) {
		std::int64_t charIndex = mInputState.caretCharIndex - 1;
		if (charIndex >= 0) {
			deleteCharacter(charIndex);
			moveCaretX(-1);
		} else {
			deleteLine(Text::DeleteLineMode::Start);
		}
	} else {
		deleteSelection();
	}
}

void TextView::deleteAction() {
	if (mInputState.selection.isSingle() || !mInputState.showSelection) {
		if (mInputState.caretCharIndex < (std::int64_t)currentLineLength()) {
			deleteCharacter(mInputState.caretCharIndex);
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

	auto drawPosition = mInputState.getDrawPosition(mRenderStyle);
	auto textY = (std::int64_t)std::floor((-drawPosition.y + mouseY) / mFont.lineHeight());
	auto relativeMousePositionX = mouseX - TextOperations::getLineNumberSpacing(mFont, mText) - drawPosition.x;

	if (textY < 0) {
		textY = 0;
	}

	if (textY >= (std::int64_t)numLines()) {
		textY = numLines() - 1;
	}

	mTextOperations.requireLineFormatted(getTextViewPort(), (std::size_t)textY);

	auto textX = (std::int64_t)mTextMetrics.getCharIndexFromScreenPosition(
		*mTextOperations.formattedText(),
		mTextOperations.formattedText()->getLine((std::size_t)textY).number,
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
				mTextOperations.updateFormattedText(getTextViewPort());
			}

			if (mouseY >= windowState.height()) {
				moveViewY((float)(windowState.height() - mouseY));
				mTextOperations.updateFormattedText(getTextViewPort());
			}

			mInputState.selection = mPotentialSelection;
			if (mInputState.selection.endY < mInputState.selection.startY) {
				std::swap(mInputState.selection.startX, mInputState.selection.endX);
				std::swap(mInputState.selection.startY, mInputState.selection.endY);
			}

			mTextOperations.requireSelectionFormatted(getTextViewPort(), mInputState.selection);
		} else {
			mPotentialSelection.startX = (std::size_t)mouseTextPosition.first;
			mPotentialSelection.startY = (std::size_t)mouseTextPosition.second;
		}
	}

	if (mInputManager.isMouseDragMove(GLFW_MOUSE_BUTTON_LEFT)) {
		if (!mSelectionStarted) {
			mSelectionStarted = true;
			mInputState.showSelection = true;
			mTextOperations.viewMoved();
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
		mTextOperations.updateFormattedText(getTextViewPort());

		auto mouseTextPosition = getMouseTextPosition();
		mInputState.caretCharIndex = mouseTextPosition.first;
		mInputState.caretLineIndex = mouseTextPosition.second;

		if (!mSelectionStarted) {
			mInputState.showSelection = false;
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

RenderViewPort TextView::getTextViewPort() const {
	auto viewPort = mViewPort;
	viewPort.width -= mRenderStyle.sideSpacing * 2;
	viewPort.height -= mRenderStyle.bottomSpacing;
	viewPort.width -= TextOperations::getLineNumberSpacing(mFont, mText);
	return viewPort;
}

namespace {
	void formattedBenchmark(const Font& font, TextFormatter& textFormatter, const RenderStyle& renderStyle, const RenderViewPort& viewPort, const Text& text) {
		for (int i = 0; i < 3; i++) {
			FormattedLines formattedLines;
			textFormatter.format(font, renderStyle, viewPort, text, formattedLines);
		}

		int n = 10;
		auto t0 = Helpers::timeNow();
		for (int i = 0; i < n; i++) {
			FormattedLines formattedLines;
			textFormatter.format(font, renderStyle, viewPort, text, formattedLines);
		}

		std::cout
			<< (Helpers::durationMicroseconds(Helpers::timeNow(), t0) / 1E3) / n << " ms"
			<< std::endl;
	}
}

void TextView::render(const WindowState& windowState, TextRender& textRender) {
	auto viewPort = getTextViewPort();
	auto lineNumberSpacing = TextOperations::getLineNumberSpacing(mFont, mText);

//	auto startTime = std::chrono::system_clock::now();
	auto drawPosition = mInputState.getDrawPosition(mRenderStyle);
	mTextOperations.updateFormattedText(viewPort);
	auto formattedText = mTextOperations.formattedText();

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

	if (mInputState.showSelection) {
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
