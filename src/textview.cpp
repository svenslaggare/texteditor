#include "textview.h"
#include "textrender.h"
#include "renderstyle.h"
#include "renderviewport.h"

#include <chrono>
#include <iostream>

TextView::TextView(const Font& font, FormatMode formatMode, const RenderViewPort& viewPort, const RenderStyle& renderStyle)
	: mFont(font), mFormatMode(formatMode), mViewPort(viewPort), mRenderStyle(renderStyle) {

}

void TextView::render(TextRender& textRender, const Text& text, glm::vec2 position) {
	auto viewPort = mViewPort;
	viewPort.width -= mRenderStyle.sideSpacing * 2;
	viewPort.height -= mRenderStyle.bottomSpacing;

//	auto startTime = std::chrono::system_clock::now();
	textRender.render(
		mFont,
		mFormatMode,
		mRenderStyle,
		viewPort,
		text,
		glm::vec2(position.x + mRenderStyle.sideSpacing, position.y + mRenderStyle.topSpacing));
//	std::cout
//		<< "Render time: " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - startTime).count() / 1E3)
//		<< "ms"
//		<< std::endl;
}
