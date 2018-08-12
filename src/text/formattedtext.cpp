#include "formattedtext.h"

FormattedLine::FormattedLine() {

}

void FormattedLine::addToken(Token token) {
	tokens.push_back(std::move(token));
}

std::size_t FormattedLine::length() const {
	std::size_t count = 0;

	for (auto& token : tokens) {
		count += token.text.size();
	}

	return count;
}

String FormattedLine::toString() const {
	String line;

	for (auto& token : tokens) {
		line += token.text;
	}

	return line;
}

std::size_t FormattedText::numLines() const {
	return mLines.size();
}

std::vector<FormattedLine>& FormattedText::lines() {
	return mLines;
}

const FormattedLine& FormattedText::getLine(std::size_t index) const {
	return mLines.at(index);
}

void FormattedText::addLine(FormattedLine tokens) {
	mLines.push_back(std::move(tokens));
}

std::size_t PartialFormattedText::numLines() const {
	return mTotalLines;
}

void PartialFormattedText::setNumLines(std::size_t count) {
	mTotalLines = count;
}

const FormattedLine& PartialFormattedText::getLine(std::size_t index) const {
	return mLines.at(index);
}

void PartialFormattedText::addLine(std::size_t index, FormattedLine tokens) {
	mLines[index] = std::move(tokens);
}

bool PartialFormattedText::hasLine(std::size_t index) const {
	return mLines.count(index) > 0;
}