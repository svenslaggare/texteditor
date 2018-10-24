#pragma once
#include "text.h"

/**
 * The format mode
 */
enum class FormatMode : std::uint8_t {
	Text,
	Code
};

/**
 * The rules formatting
 */
class FormatterRules {
public:
	virtual ~FormatterRules() = default;

	/**
	 * Returns the mode
	 */
	virtual FormatMode mode() const = 0;

	/**
	 * Indicates if the given string is a keyword
	 * @param string The string
	 */
	virtual bool isKeyword(const String& string) const = 0;

	/**
	 * The start of a line comment
	 */
	virtual const String& lineCommentStart() const = 0;

	/**
	 * The start of a block comment
	 */
	virtual const String& blockCommentStart() const = 0;

	/**
	 * The end of a block comment
	 */
	virtual const String& blockCommentEnd() const = 0;

	/**
	 * Indicates if the given char is a string delimiter
	 * @param current The character
	 */
	virtual bool isStringDelimiter(Char current) const = 0;
};