#pragma once
#include <string>
#include <vector>
#include <functional>

class Font;
class RenderStyle;

/**
 * Represents text
 */
class Text {
private:
	std::vector<std::string> mLines;
	std::size_t mNumLines = 0;
	std::size_t mVersion = 0;
public:
	/**
	 * Creates a new text
	 * @param text The raw text
	 */
	Text(std::string text);

	/**
	 * Applies the given function to each character in the text
	 * @param apply The function to apply
	 */
	void forEach(std::function<void (std::size_t, char)> apply) const;

	/**
	 * Returns the current version
	 */
	std::size_t version() const;

	/**
	 * Indicates if the text has changed
	 * @param version The version to check for. If changed, updates version
	 */
	bool hasChanged(std::size_t& version) const;

	/**
	 * Inserts the given character at the given index at the given line
	 * @param lineNumber The line to insert at
	 * @param index The index
	 * @param character The character
	 */
	void insertAt(std::size_t lineNumber, std::size_t index, char character);

	/**
	 * Deletes the character at the given index at the given line
	 * @param lineNumber The line to insert at
	 * @param index The index
	 */
	void deleteAt(std::size_t lineNumber, std::size_t index);

	/**
	 * Returns the number of lines
	 */
	std::size_t numLines() const;
};