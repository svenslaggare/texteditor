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

const FormattedText& Text::getFormatted(const Font& font, FormatMode formatMode, const RenderStyle& renderStyle, const RenderViewPort& viewPort) const {
	if (mLastViewPort.width != viewPort.width
		|| mLastViewPort.height != viewPort.height
		|| mLastViewPort.position != viewPort.position) {
		mFormattedText = {};
		mLastViewPort = viewPort;

//		formattedBenchmark(font, formatMode, renderStyle, viewPort, mRaw);

		TextFormatter textFormatter(formatMode);
		auto t0 = Helpers::timeNow();
		textFormatter.format(font, renderStyle, viewPort, mRaw, mFormattedText);
		std::cout
			<< "Formatted text (lines = " << mFormattedText.numLines() << ") in "
	  		<< (Helpers::durationMicroseconds(Helpers::timeNow(), t0) / 1E3) << " ms"
			<< std::endl;
	}

	return mFormattedText;
}
