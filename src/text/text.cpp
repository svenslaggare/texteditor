#include <iostream>
#include <chrono>
#include "text.h"
#include "../helpers.h"

void TextSelection::setSingle(std::size_t x, std::size_t y) {
	startChar = x;
	startLine = y;
	endChar = x;
	endLine = y;
}

bool TextSelection::isSingle() const {
	return startChar == endChar && startLine == endLine;
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

void Text::insertAt(std::size_t lineIndex, std::size_t charIndex, Char character) {
	auto startTime = Helpers::timeNow();
	mVersion++;

	auto& line = mLines.at(lineIndex);
	auto maxIndex = (std::size_t)std::max((std::int64_t)line.size(), 0L);
	charIndex = std::min(charIndex, maxIndex);
	line.insert(line.begin() + charIndex, character);

	std::cout << "Inserted character in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}

void Text::insertAt(std::size_t lineIndex, std::size_t charIndex, const String& str) {
	auto startTime = Helpers::timeNow();
	mVersion++;

	auto& line = mLines.at(lineIndex);
	auto maxIndex = (std::size_t)std::max((std::int64_t)line.size(), 0L);
	charIndex = std::min(charIndex, maxIndex);
	line.insert(charIndex, str);

	std::cout << "Inserted string in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}

void Text::insertLine(std::size_t lineIndex, const String& line) {
	auto startTime = Helpers::timeNow();
	mVersion++;

	mLines.insert(mLines.begin() + lineIndex + 1, line);
	std::cout << "Insert line in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}

void Text::insertText(std::size_t lineIndex, std::size_t charIndex, const Text& text) {
	auto startTime = Helpers::timeNow();
	mVersion++;

	insertAt(lineIndex, charIndex, text.getLine(0));
	mLines.insert(mLines.begin() + lineIndex + 1, text.mLines.begin() + 1, text.mLines.end());

	std::cout << "Insert text in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}

void Text::deleteAt(std::size_t lineIndex, std::size_t charIndex) {
	auto startTime = Helpers::timeNow();
	mVersion++;

	auto& line = mLines.at(lineIndex);
	auto maxIndex = (std::size_t)std::max((std::int64_t)line.size(), 0L);
	charIndex = std::min(charIndex, maxIndex);
	line.erase(line.begin() + charIndex);

	std::cout << "Deleted character in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}

void Text::splitLine(std::size_t lineNumber, std::size_t charIndex) {
	auto startTime = Helpers::timeNow();
	mVersion++;

	auto& line = mLines.at(lineNumber);
	auto afterSplit = line.substr(charIndex);
	line.erase(line.begin() + charIndex, line.end());
	mLines.insert(mLines.begin() + lineNumber + 1, afterSplit);

	std::cout << "Split line in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
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
	if (textSelection.startLine == textSelection.endLine) {
		auto& line = mLines.at(textSelection.startLine);
		mLines.at(textSelection.startLine) = line.substr(0, textSelection.startChar) + line.substr(std::min(textSelection.endChar + 1, line.size()));
		deleteSelectionData.startDeleteLineIndex = textSelection.startLine;
		deleteSelectionData.endDeleteLineIndex = textSelection.endLine;
	} else {
		std::size_t deleteLineStartIndex = textSelection.startLine + 1;
		std::size_t deleteLineEndIndex = textSelection.endLine;

		auto& lastLine = mLines.at(textSelection.endLine);
		auto lastLineRemoveIndex = std::min(textSelection.endChar + 1, lastLine.size());
		bool deleteLastLine = false;
		if (lastLineRemoveIndex == lastLine.size()) {
			deleteLastLine = true;
		} else {
			deleteLineEndIndex--;
		}

		mLines.at(textSelection.startLine) = mLines.at(textSelection.startLine).substr(0, textSelection.startChar);
		if (!deleteLastLine) {
			mLines.at(textSelection.endLine) = lastLine.substr(lastLineRemoveIndex);
		}

		deleteSelectionData.startDeleteLineIndex = deleteLineStartIndex;
		deleteSelectionData.endDeleteLineIndex = deleteLineEndIndex;

		mLines.erase(mLines.begin() + deleteLineStartIndex, mLines.begin() + deleteLineEndIndex + 1);
	}

	std::cout << "Deleted selection in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;

	return deleteSelectionData;
}
