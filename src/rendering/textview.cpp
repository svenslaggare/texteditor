#include "textview.h"
#include "textrender.h"
#include "renderstyle.h"
#include "renderviewport.h"
#include "font.h"

#include "../text/text.h"

#include "../inputmanager.h"
#include "../windowstate.h"

#include <chrono>
#include <iostream>
#include <algorithm>

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

}

const LineTokens& TextView::currentLine() const {
	return mFormattedText.getLine((std::size_t)mInputState.caretPositionY);
}

void TextView::updateViewMovement(const WindowState& windowState) {
	auto viewPort = getTextViewPort();
	const auto lineHeight = mFont.lineHeight();
	const auto lineWidth = mFont['A'].advanceX;

	const auto pageMoveSpeed = viewPort.height;
	if (mInputManager.isKeyPressed(GLFW_KEY_PAGE_UP)) {
		mInputState.viewPosition.y += pageMoveSpeed;
	} else if (mInputManager.isKeyPressed(GLFW_KEY_PAGE_DOWN)) {
		mInputState.viewPosition.y -= pageMoveSpeed;
	}

	auto moveCaretY = [&](int diff) {
		mInputState.caretPositionY += diff;
		mInputState.caretPositionY = std::max(mInputState.caretPositionY, 0L);

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

		mInputState.caretPositionX = std::min(
			(std::size_t)mInputState.caretPositionX,
			currentLine().length());
	};

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

		mInputState.caretPositionX += caretPositionDiffX;

		if (caretPositionDiffX == -1) {
			if (mInputState.caretPositionX < 0L) {
				moveCaretY(-1);

				if (mFormattedText.numLines() > 0) {
					mInputState.caretPositionX = (std::int64_t)currentLine().length();
				} else {
					mInputState.caretPositionX = 0;
				}
			}
		} else {
			if (mFormattedText.numLines() > 0) {
				if ((std::size_t) mInputState.caretPositionX > currentLine().length()) {
					moveCaretY(1);
					mInputState.caretPositionX = 0;
				}
			}
		}

		auto caretScreenPositionX = -std::max(mInputState.caretPositionX + caretPositionDiffX, 0L) * lineWidth;
		if (caretScreenPositionX < mInputState.viewPosition.x - viewPort.width) {
			mInputState.viewPosition.x -= caretPositionDiffX * lineWidth;
		}

		if (caretScreenPositionX > mInputState.viewPosition.x) {
			mInputState.viewPosition.x -= caretPositionDiffX * lineWidth;
		}
	}

	if (windowState.hasScrolled()) {
		mInputState.viewPosition.y += mScrollSpeed * lineHeight * windowState.scrollY();

		if (mInputState.viewPosition.y > 0) {
			mInputState.viewPosition.y = 0;
		}
	}
}

void TextView::updateEditing(const WindowState& windowState) {
	auto moveCaretX = [&](int diff) {
		mInputState.caretPositionX += diff;
		updateFormattedText(getTextViewPort());
	};

	auto getLineAndOffset = [&](int offsetX = 0) {
		auto& line = currentLine();
		std::size_t lineIndex = line.number;
		auto offset = (std::int64_t)line.offsetFromTextLine + mInputState.caretPositionX + offsetX;
		return std::make_pair(lineIndex, offset);
	};

	auto insertCharacter = [&](char current) {
		auto lineAndOffset = getLineAndOffset();
		mText.insertAt(lineAndOffset.first, (std::size_t)lineAndOffset.second, current);
		moveCaretX(1);
	};

	for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
		if (mInputManager.isKeyPressed(key)) {
			insertCharacter((char)('a' + (key - GLFW_KEY_A)));
		}
	}

	for (int key = GLFW_KEY_0; key <= GLFW_KEY_9; key++) {
		if (mInputManager.isKeyPressed(key)) {
			insertCharacter((char)('0' + (key - GLFW_KEY_0)));
		}
	}

	if (mInputManager.isKeyPressed(GLFW_KEY_SPACE)) {
		insertCharacter(' ');
	}

	if (mInputManager.isKeyPressed(GLFW_KEY_BACKSPACE)) {
		auto lineAndOffset = getLineAndOffset(-1);
		if (lineAndOffset.second >= 0) {
			mText.deleteAt(lineAndOffset.first, (std::size_t)lineAndOffset.second);
			moveCaretX(-1);
		} else {

		}
	}

	if (mInputManager.isKeyPressed(GLFW_KEY_DELETE)) {
		auto lineAndOffset = getLineAndOffset();
		if (lineAndOffset.second < currentLine().length()) {
			mText.deleteAt(lineAndOffset.first, (std::size_t)lineAndOffset.second);
		} else {

		}
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
	void formattedBenchmark(const Font& font, FormatMode formatMode, const RenderStyle& renderStyle, const RenderViewPort& viewPort, const std::string& text) {
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
	if (mLastViewPort.width != viewPort.width
		|| mLastViewPort.height != viewPort.height
		|| mLastViewPort.position != viewPort.position
		|| mText.hasChanged(mTextVersion)) {
		mFormattedText = {};
		mLastViewPort = viewPort;

//		formattedBenchmark(font, formatMode, renderStyle, viewPort, mRaw);

		TextFormatter textFormatter(mFormatMode);
		auto t0 = Helpers::timeNow();
		textFormatter.format(mFont, mRenderStyle, viewPort, mText, mFormattedText);
		std::cout
			<< "Formatted text (lines = " << mFormattedText.numLines() << ") in "
			<< (Helpers::durationMicroseconds(Helpers::timeNow(), t0) / 1E3) << " ms"
			<< std::endl;
	}
}

void TextView::render(TextRender& textRender) {
	auto viewPort = getTextViewPort();
	auto lineNumberSpacing = getLineNumberSpacing();

//	auto startTime = std::chrono::system_clock::now();
	auto drawPosition = glm::vec2(
		mInputState.viewPosition.x + mRenderStyle.sideSpacing,
		mInputState.viewPosition.y + mRenderStyle.topSpacing);

	updateFormattedText(viewPort);
	textRender.renderLineNumbers(
		mFont,
		mRenderStyle,
		viewPort,
		mFormattedText,
		drawPosition);

	textRender.render(
		mFont,
		mRenderStyle,
		viewPort,
		mFormattedText,
		drawPosition + glm::vec2(lineNumberSpacing, 0.0f));

	if (mDrawCaret) {
		textRender.renderCaret(
			mFont,
			mRenderStyle,
			viewPort,
			mFormattedText,
			{ lineNumberSpacing + mRenderStyle.sideSpacing, mRenderStyle.topSpacing },
			mInputState);
	}

//	std::cout
//		<< "Render time: " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - startTime).count() / 1E3)
//		<< "ms"
//		<< std::endl;
}
