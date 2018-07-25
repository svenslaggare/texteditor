#include <iostream>
#include <chrono>
#include "text.h"
#include "../helpers.h"

Text::Text(std::string text) {
	std::string line;
	for (auto c : text) {
		if (c == '\n') {
			mLines.push_back(std::move(line));
			line = "";
		} else {
			line += c;
		}
	}

	if (!line.empty()) {
		mLines.push_back(std::move(line));
	}
}

void Text::forEach(std::function<void(std::size_t, char)> apply) const {
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

std::size_t Text::numLines() const {
	return mLines.size();
}

std::size_t Text::version() const {
	return mVersion;
}

const std::string& Text::getLine(std::size_t index) const {
	return mLines.at(index);
}

bool Text::hasChanged(std::size_t& version) const {
	if (mVersion != version) {
		version = mVersion;
		return true;
	}

	return false;
}

void Text::insertAt(std::size_t lineNumber, std::size_t index, char character) {
	auto startTime = Helpers::timeNow();
	mVersion++;
	auto& line = mLines.at(lineNumber);
	auto maxIndex = (std::size_t)std::max((std::int64_t)line.size(), 0L);
	index = std::min(index, maxIndex);
	line.insert(line.begin() + index, character);

	std::cout << "Inserted character in " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
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
	std::string afterSplit = line.substr(index);
	line.erase(line.begin() + index, line.end());
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
