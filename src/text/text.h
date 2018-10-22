#pragma once
#include <string>
#include <vector>
#include <functional>

class Font;
struct RenderStyle;

//using Char = char;
//using String = std::string;
using Char = char16_t;
using String = std::u16string;

/**
 * Represents a text selection
 */
struct TextSelection {
	std::size_t startX = 0;
	std::size_t startY = 0;
	std::size_t endX = 0;
	std::size_t endY = 0;

	/**
	 * Set the selection to the given single character
	 * @param x The x coordinate
	 * @param y The y coordinate
	 */
	void setSingle(std::size_t x, std::size_t y);

	/**
	 * Indicates if the selection only covers a single character
	 */
	bool isSingle() const;
};

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
	 * Applies the given function to each line in the given text
	 * @param apply The function to apply
	 */
	void forEachLine(std::function<void (const String&)> apply) const;

	/**
	 * Indicates if the text has changed
	 * @param version The version to check for. If changed, updates version
	 */
	bool hasChanged(std::size_t& version) const;

	/**
	 * Inserts the given character at the given index at the given line
	 * @param lineIndex The line to insert at
	 * @param charIndex The char index
	 * @param character The character
	 */
	void insertAt(std::size_t lineIndex, std::size_t charIndex, Char character);

	/**
	 * Inserts the given string at the given index at the given line
	 * @param lineIndex The line to insert at
	 * @param charIndex The char index
	 * @param str The string to insert
	 */
	void insertAt(std::size_t lineIndex, std::size_t charIndex, const String& str);

	/**
	 * Inserts a line after the given line
	 * @param lineIndex The number of the line
	 * @param line The line
	 */
	void insertLine(std::size_t lineIndex, const String& line);

	/**
	 * Inserts the given text
	 * @param lineIndex The line number to start inserting at
	 * @param charIndex The char index to start inserting at
	 * @param text The text
	 */
	void insertText(std::size_t lineIndex, std::size_t charIndex, const Text& text);

	/**
	 * Deletes the character at the given index at the given line
	 * @param lineIndex The line to insert at
	 * @param charIndex The index
	 */
	void deleteAt(std::size_t lineIndex, std::size_t charIndex);

	/**
	 * Splits the given line
	 * @param lineNumber The line to split
	 * @param charIndex The index at the line to perform the split
	 */
	void splitLine(std::size_t lineNumber, std::size_t charIndex);

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

	/**
	 * Data created when deleted selection
	 */
	struct DeleteSelectionData {
		std::size_t startDeleteLineIndex = 0;
		std::size_t endDeleteLineIndex = 0;
	};

	/**
	 * Deletes the given text selection
	 * @param textSelection The text selection
	 */
	DeleteSelectionData deleteSelection(const TextSelection& textSelection);
};