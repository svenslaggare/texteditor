#pragma once
#include <string>
#include <chrono>

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

namespace Helpers {
	/**
	 * Returns the content of the given file as string
	 * @param fileName The name of the file
	 * @return
	 */
	std::string readFileAsText(const std::string& fileName);

	/**
	 * Replaces all occurrences of from in str with to returning a new string
	 * @param str The string
	 * @param from The characters to search from
	 * @param to The replace characters
	 * @return The new string
	 */
	std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

	/*
	 * Returns the current time
	 */
	TimePoint timeNow();

	/**
	 * Returns the duration in microseconds between x and y
	 * @param x The first point in time
	 * @param y The second point in time
	 */
	long duration(TimePoint x, TimePoint y);

	/**
	 * Returns the duration in milliseconds between x and y
	 * @param x The first point in time
	 * @param y The second point in time
	 */
	double durationMS(TimePoint x, TimePoint y);
}