#include "helpers.h"
#include <fstream>
#include <codecvt>
#include <locale>
#include <iostream>

std::string Helpers::readFileAsUTF8Text(const std::string& fileName) {
	std::ifstream fileStream(fileName);
	if (!fileStream.is_open()) {
		throw std::runtime_error("The file '" + fileName + "' does not exist.");
	}

	std::string text;

	fileStream.seekg(0, std::ios::end);
	text.reserve(fileStream.tellg());
	fileStream.seekg(0, std::ios::beg);

	text.assign(std::istreambuf_iterator<char>(fileStream),
				std::istreambuf_iterator<char>());

	return text;
}

std::u16string Helpers::readFileAsUTF16Text(const std::string& fileName) {
	auto text = readFileAsUTF8Text(fileName);
	std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> cv;
	return cv.from_bytes(text);
}

TimePoint Helpers::timeNow() {
	return std::chrono::system_clock::now();
}

long Helpers::durationMicroseconds(TimePoint x, TimePoint y) {
	return std::chrono::duration_cast<std::chrono::microseconds>(x - y).count();
}

double Helpers::durationMilliseconds(TimePoint x, TimePoint y) {
	return durationMicroseconds(x, y) / 1E3;
}

Timing::Timing(const std::string& before)
	: before(before) {

}

Timing::~Timing() {
	std::cout << before << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << " ms" << std::endl;
}
