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
			mNumLines++;
		} else {
			line += c;
		}
	}

	mLines.push_back(std::move(line));
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
	return mNumLines;
}

std::size_t Text::version() const {
	return mVersion;
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
