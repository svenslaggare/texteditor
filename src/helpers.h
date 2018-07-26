#pragma once
#include <string>
#include <chrono>

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

namespace Helpers {
	/**
	 * Returns the content of the given file as UTF-8 string
	 * @param fileName The name of the file
	 */
	std::string readFileAsUTF8Text(const std::string& fileName);

	/**
	 * Returns the content of the given file as UTF-16 string
	 * @param fileName The name of the file
	 */
	std::u16string readFileAsUTF16Text(const std::string& fileName);

	template<typename T>
	struct FileTextReader {
		T read(const std::string& fileName);
	};

	template<>
	struct FileTextReader<std::string> {
		std::string read(const std::string& fileName) {
			return readFileAsUTF8Text(fileName);
		}
	};

	template<>
	struct FileTextReader<std::u16string> {
		std::u16string read(const std::string& fileName) {
			return readFileAsUTF16Text(fileName);
		}
	};

	/**
	 * Returns the content of the given file as string
	 * @param fileName The name of the file
	 * @tparam T The type of the string
	 */
	template<typename T>
	T readFileAsText(const std::string& fileName) {
		return FileTextReader<T>().read(fileName);
	}

	/*
	 * Returns the current time
	 */
	TimePoint timeNow();

	/**
	 * Returns the duration in microseconds between x and y
	 * @param x The first point in time
	 * @param y The second point in time
	 */
	long durationMicroseconds(TimePoint x, TimePoint y);

	/**
	 * Returns the duration in milliseconds between x and y
	 * @param x The first point in time
	 * @param y The second point in time
	 */
	double durationMilliseconds(TimePoint x, TimePoint y);
}