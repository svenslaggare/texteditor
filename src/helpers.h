#pragma once
#include <string>

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
}