#include "textview.h"
#include "textrender.h"
#include "renderstyle.h"
#include "renderviewport.h"
#include "../text/text.h"
#include "font.h"
#include "../inputmanager.h"

#include <chrono>
#include <iostream>
#include <algorithm>

TextView::TextView(const Font& font, FormatMode formatMode, const RenderViewPort& viewPort, const RenderStyle& renderStyle)
	: mFont(font), mFormatMode(formatMode), mViewPort(viewPort), mRenderStyle(renderStyle) {

}

void TextView::render(TextRender& textRender, const Text& text, const InputState& inputState, bool drawCaret) {
	auto viewPort = mViewPort;
	viewPort.width -= mRenderStyle.sideSpacing * 2;
	viewPort.height -= mRenderStyle.bottomSpacing;

//	auto startTime = std::chrono::system_clock::now();
	auto drawPosition = glm::vec2(
		inputState.viewPosition.x + mRenderStyle.sideSpacing,
		inputState.viewPosition.y + mRenderStyle.topSpacing);

	auto lineNumberSpacing = (std::to_string(text.numLines()).size() + 1) * mFont['A'].advanceX;
	viewPort.width -= lineNumberSpacing;

	auto& formattedText = text.getFormatted(mFont, mFormatMode, mRenderStyle, viewPort);
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

	if (drawCaret) {
		textRender.renderCaret(
			mFont,
			mRenderStyle,
			viewPort,
			{ lineNumberSpacing + mRenderStyle.sideSpacing, mRenderStyle.topSpacing },
			inputState);
	}

//	std::cout
//		<< "Render time: " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - startTime).count() / 1E3)
//		<< "ms"
//		<< std::endl;
}
