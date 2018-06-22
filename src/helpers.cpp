#include "helpers.h"
#include <fstream>

std::string Helpers::readFileAsText(const std::string& fileName) {
	std::ifstream configStream(fileName);
	if (!configStream.is_open()) {
		throw std::runtime_error("The file '" + fileName + "' does not exist.");
	}

	std::string text;

	configStream.seekg(0, std::ios::end);
	text.reserve(configStream.tellg());
	configStream.seekg(0, std::ios::beg);

	text.assign(std::istreambuf_iterator<char>(configStream),
				std::istreambuf_iterator<char>());

	return text;
}

std::string Helpers::replaceAll(const std::string& str, const std::string& from, const std::string& to) {
	auto result = str;

	std::size_t start_pos = 0;
	while((start_pos = result.find(from, start_pos)) != std::string::npos) {
		result.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}

	return result;
}

TimePoint Helpers::timeNow() {
	return std::chrono::system_clock::now();
}

long Helpers::duration(TimePoint x, TimePoint y) {
	return std::chrono::duration_cast<std::chrono::microseconds>(x - y).count();
}
