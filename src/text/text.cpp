#include <iostream>
#include <chrono>
#include "text.h"
#include "../helpers.h"

void TextSelection::setSingle(std::size_t x, std::size_t y) {
	startX = x;
	startY = y;
	endX = x;
	endY = y;
}

bool TextSelection::isSingle() const {
	return startX == endX && startY == endY;
}

Text::Text(String text) {
	if (!text.empty()) {
		String line;
		for (auto c : text) {
			if (c == '\n') {
				mLines.push_back(std::move(line));
				line = {};
			} else {
				line += c;
			}
		}

		if (!line.empty()) {
			mLines.push_back(std::move(line));
		}
	} else {
		mLines.emplace_back();
	}
}

void Text::forEach(std::function<void(std::size_t, Char)> apply) const {
	std::size_t i = 0;
	for (auto& line : mLines) {
		for (auto& c : line) {
			apply(i, c);
			i++;
		}

		apply(i, '\n');
		i++;
	}
}

void Text::forEachLine(std::function<void(const String&)> apply) const {
	for (auto& line : mLines) {
		apply(line);
	}
}

std::size_t Text::numLines() const {
	return mLines.size();
}

std::size_t Text::version() const {
	return mVersion;
}

const String& Text::getLine(std::size_t index) const {
	return mLines.at(index);
}

bool Text::hasChanged(std::size_t& version) const {
	if (mVersion != version) {
		version = mVersion;
		return true;
	}

	return false;
}

void Text::insertAt(std::size_t lineNumber, std::size_t index, Char character) {
	auto startTime = Helpers::timeNow();
	mVersion++;
	auto& line = mLines.at(lineNumber);
	auto maxIndex = (std::size_t)std::max((std::int64_t)line.size(), 0L);
	index = std::min(index, maxIndex);
	line.insert(line.begin() + index, character);

	std::cout << "Inserted character in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}

void Text::insertAt(std::size_t lineNumber, std::size_t index, const String& str) {
	auto startTime = Helpers::timeNow();
	mVersion++;
	auto& line = mLines.at(lineNumber);
	auto maxIndex = (std::size_t)std::max((std::int64_t)line.size(), 0L);
	index = std::min(index, maxIndex);
	line.insert(index, str);

	std::cout << "Inserted string in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}

void Text::deleteAt(std::size_t lineNumber, std::size_t index) {
	auto startTime = Helpers::timeNow();
	mVersion++;
	auto& line = mLines.at(lineNumber);
	auto maxIndex = (std::size_t)std::max((std::int64_t)line.size(), 0L);
	index = std::min(index, maxIndex);
	line.erase(line.begin() + index);

	std::cout << "Deleted character in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}

void Text::splitLine(std::size_t lineNumber, std::size_t index) {
	auto startTime = Helpers::timeNow();
	mVersion++;

	auto& line = mLines.at(lineNumber);
	auto afterSplit = line.substr(index);
	line.erase(line.begin() + index, line.end());
	mLines.insert(mLines.begin() + lineNumber + 1, afterSplit);

	std::cout << "Split line in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}

void Text::insertLine(std::size_t lineNumber, const String& line) {
	auto startTime = Helpers::timeNow();
	mVersion++;

	mLines.insert(mLines.begin() + lineNumber + 1, line);
	std::cout << "Insert line in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}

Text::DeleteLineDiff Text::deleteLine(std::size_t lineNumber, DeleteLineMode mode) {
	auto startTime = Helpers::timeNow();
	mVersion++;
	DeleteLineDiff diff;

	if (mode == DeleteLineMode::Start) {
		if (lineNumber > 0) {
			diff.caretX = mLines.at(lineNumber - 1).length();
			mLines.at(lineNumber - 1) += mLines.at(lineNumber);
		}

		mLines.erase(mLines.begin() + lineNumber);
	} else {
		if (lineNumber + 1 < numLines()) {
			mLines.at(lineNumber) += mLines.at(lineNumber + 1);
			mLines.erase(mLines.begin() + lineNumber + 1);
		}
	}

	std::cout << "Deleted line in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
	return diff;
}

Text::DeleteSelectionData Text::deleteSelection(const TextSelection& textSelection) {
	auto startTime = Helpers::timeNow();
	mVersion++;
	DeleteSelectionData deleteSelectionData;

	if (textSelection.startY == textSelection.endY) {
		auto& line = mLines.at(textSelection.startY);
		mLines.at(textSelection.startY) = line.substr(0, textSelection.startX) + line.substr(std::min(textSelection.endX + 1, line.size()));
		deleteSelectionData.startDeleteLineIndex = textSelection.startY;
		deleteSelectionData.endDeleteLineIndex = textSelection.endY;
	} else {
		std::size_t deleteLineStartIndex = textSelection.startY + 1;
		std::size_t deleteLineEndIndex = textSelection.endY;

		auto& lastLine = mLines.at(textSelection.endY);
		auto lastLineRemoveIndex = std::min(textSelection.endX + 1, lastLine.size());
		bool deleteLastLine = false;
		if (lastLineRemoveIndex == lastLine.size()) {
			deleteLastLine = true;
		} else {
			deleteLineEndIndex--;
		}

		mLines.at(textSelection.startY) = mLines.at(textSelection.startY).substr(0, textSelection.startX);
		if (!deleteLastLine) {
			mLines.at(textSelection.endY) = lastLine.substr(lastLineRemoveIndex);
		}

		deleteSelectionData.startDeleteLineIndex = deleteLineStartIndex;
		deleteSelectionData.endDeleteLineIndex = deleteLineEndIndex;

		mLines.erase(mLines.begin() + deleteLineStartIndex, mLines.begin() + deleteLineEndIndex + 1);
	}

	std::cout << "Deleted selection in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;

	return deleteSelectionData;
}
