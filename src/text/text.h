#pragma once

#include <string>
#include <vector>
#include <functional>

class Font;
class RenderStyle;

using Char = char;
using String = std::string;

/**
 * Represents text
 */
class Text {
private:
	std::vector<String> mLines;
	std::size_t mVersion = 0;
public:
	/**
	 * Creates a new text
	 * @param text The raw text
	 */
	Text(String text);

	/**
	 * Returns the current version
	 */
	std::size_t version() const;

	/**
	 * Returns the number of lines
	 */
	std::size_t numLines() const;

	/**
	 * Returns the given line
	 * @param index The index
	 */
	const String& getLine(std::size_t index) const;

	/**
	 * Applies the given function to each character in the text
	 * @param apply The function to apply
	 */
	void forEach(std::function<void (std::size_t, Char)> apply) const;

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
	void insertAt(std::size_t lineNumber, std::size_t index, Char character);

	/**
	 * Deletes the character at the given index at the given line
	 * @param lineNumber The line to insert at
	 * @param index The index
	 */
	void deleteAt(std::size_t lineNumber, std::size_t index);

	/**
	 * Splits the given line
	 * @param lineNumber The line to split
	 * @param index The index at the line to perform the split
	 */
	void splitLine(std::size_t lineNumber, std::size_t index);

	enum class DeleteLineMode {
		Start,
		End
	};

	struct DeleteLineDiff {
		std::int64_t caretX = 0;
	};

	/**
	 * Deletes the given line
	 * @param lineNumber The line to remove
	 * @param mode How to delete the line
	 */
	DeleteLineDiff deleteLine(std::size_t lineNumber, DeleteLineMode mode);
};