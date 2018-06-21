#include <iostream>
#include <chrono>
#include "text.h"
#include "textformatter.h"

Text::Text(std::string text)
	: mRaw(std::move(text)) {

}

const FormattedText& Text::getFormatted(const Font& font, FormatMode formatMode, const RenderStyle& renderStyle, const RenderViewPort& viewPort) const {
	if (mLastViewPort.width != viewPort.width
		|| mLastViewPort.height != viewPort.height
		|| mLastViewPort.position != viewPort.position) {
		mFormattedText.lines.clear();
		mLastViewPort = viewPort;
		TextFormatter textFormatter(formatMode);
		auto t0 = std::chrono::system_clock::now();
		textFormatter.format(font, renderStyle, viewPort, mRaw, mFormattedText);
		std::cout
			<< "Formatted text in "
	  		<< (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - t0).count() / 1E3) << " ms"
			<< std::endl;
	}

	return mFormattedText;
}
