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
				   const Text& text)
	: mWindow(window),
	  mFont(font),
	  mFormatMode(formatMode),
	  mViewPort(viewPort),
	  mRenderStyle(renderStyle),
	  mInputManager(window),
	  mText(text) {

}

void TextView::updateInput(const WindowState& windowState) {
	auto viewPort = getTextViewPort();
	const auto lineHeight = mFont.lineHeight();
	const auto lineWidth = mFont['A'].advanceX;

	const auto pageMoveSpeed = viewPort.height;
	if (mInputManager.isKeyPressed(GLFW_KEY_PAGE_UP)) {
		mInputState.viewPosition.y += pageMoveSpeed;
	} else if (mInputManager.isKeyPressed(GLFW_KEY_PAGE_DOWN)) {
		mInputState.viewPosition.y -= pageMoveSpeed;
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

		mInputState.caretPositionY += caretPositionDiffY;
		mInputState.caretPositionY = std::max(mInputState.caretPositionY, 0L);

		auto caretScreenPositionY = -std::max(mInputState.caretPositionY + caretPositionDiffY, 0L) * lineHeight;
		if (caretScreenPositionY < mInputState.viewPosition.y - viewPort.height) {
			mInputState.viewPosition.y -= caretPositionDiffY * lineHeight;
		}

		if (caretScreenPositionY > mInputState.viewPosition.y) {
			mInputState.viewPosition.y -= caretPositionDiffY * lineHeight;
		}

		if (!(-mInputState.viewPosition.y + viewPort.height >= -caretScreenPositionY && -mInputState.viewPosition.y <= -caretScreenPositionY)) {
			mInputState.viewPosition.y = -mInputState.caretPositionY * lineHeight + viewPort.height / 2.0f;
		}

		if (mInputState.viewPosition.y > 0) {
			mInputState.viewPosition.y = 0;
		}
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
		mInputState.caretPositionX = std::max(mInputState.caretPositionX, 0L);

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

void TextView::render(TextRender& textRender) {
	auto viewPort = getTextViewPort();
	auto lineNumberSpacing = getLineNumberSpacing();

//	auto startTime = std::chrono::system_clock::now();
	auto drawPosition = glm::vec2(
		mInputState.viewPosition.x + mRenderStyle.sideSpacing,
		mInputState.viewPosition.y + mRenderStyle.topSpacing);

	auto& formattedText = mText.getFormatted(mFont, mFormatMode, mRenderStyle, viewPort);
	textRender.renderLineNumbers(
		mFont,
		mRenderStyle,
		viewPort,
		formattedText.numLines(),
		drawPosition);

	textRender.render(
		mFont,
		mRenderStyle,
		viewPort,
		formattedText,
		drawPosition + glm::vec2(lineNumberSpacing, 0.0f));

	if (mDrawCaret) {
		textRender.renderCaret(
			mFont,
			mRenderStyle,
			viewPort,
			{ lineNumberSpacing + mRenderStyle.sideSpacing, mRenderStyle.topSpacing },
			mInputState);
	}

//	std::cout
//		<< "Render time: " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - startTime).count() / 1E3)
//		<< "ms"
//		<< std::endl;
}
