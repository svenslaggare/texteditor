#include <iostream>
#include <chrono>
#include "text.h"
#include "textformatter.h"
#include "../helpers.h"

Text::Text(std::string text)
	: mRaw(std::move(text)) {
	for (auto c : mRaw) {
		if (c == '\n') {
			mNumLines++;
		}
	}
}

std::size_t Text::numLines() const {
	return mNumLines;
}

const FormattedText& Text::getFormatted(const Font& font, FormatMode formatMode, const RenderStyle& renderStyle, const RenderViewPort& viewPort) const {
	if (mLastViewPort.width != viewPort.width
		|| mLastViewPort.height != viewPort.height
		|| mLastViewPort.position != viewPort.position) {
		mFormattedText = {};
		mLastViewPort = viewPort;

		TextFormatter textFormatter(formatMode);
		auto t0 = Helpers::timeNow();
		textFormatter.format(font, renderStyle, viewPort, mRaw, mFormattedText);
		std::cout
			<< "Formatted text (lines = " << mFormattedText.numLines() << ") in "
	  		<< (Helpers::duration(Helpers::timeNow(), t0) / 1E3) << " ms"
			<< std::endl;
	}

	return mFormattedText;
}
