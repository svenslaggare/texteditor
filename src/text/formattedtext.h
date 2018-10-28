#pragma once
#include "text.h"

#include <list>
#include <unordered_map>

/**
 * The type of a token
 */
enum class TokenType : std::uint8_t {
	Text,
	Number,
	Keyword,
	String,
	Comment
};

/**
 * Represents a token
 */
struct Token {
	TokenType type = TokenType::Text;
	String text;
};

/**
 * Represents a formatted line
 */
struct FormattedLine {
	std::size_t number = 0;
	std::vector<Token> tokens;
	std::size_t offsetFromTextLine = 0;
	bool isContinuation = false;
	std::int64_t reformatAmount = 0;
	std::int64_t reformatStartSearch = 0;
	bool mayRequireSearch = false;

	FormattedLine();

	/**
	 * Adds the given token
	 * @param token The token
	 */
	void addToken(Token token);

	/**
	 * Returns the number of characters on the line
	 */
	std::size_t length() const;

	/**
	 * Returns a string representation of the current line
	 */
	String toString() const;
};

/**
 * Represents a base class for formatted text
 */
class BaseFormattedText {
public:
	virtual ~BaseFormattedText() = default;

	/**
	 * Returns the number of lines
	 */
	virtual std::size_t numLines() const = 0;

	/**
	 * Returns the formatting for the given line
	 * @param index The index
	 */
	virtual const FormattedLine& getLine(std::size_t index) const = 0;
};

/**
 * Represents formatted text
 */
class FormattedText : public BaseFormattedText {
private:
	std::vector<FormattedLine> mLines;
public:
	/**
	 * Returns the number of lines
	 */
	std::size_t numLines() const override;

	/**
	 * Returns the underlying lines
	 */
	std::vector<FormattedLine>& lines();

	/**
	 * Returns the formatting for the given line
	 * @param index The index
	 */
	const FormattedLine& getLine(std::size_t index) const override;

	/**
	 * Adds the given line
	 * @param tokens The tokens on the line
	 */
	void addLine(FormattedLine tokens);
};

/**
 * Represents a partially formatted text
 */
class PartialFormattedText : public BaseFormattedText {
private:
	std::size_t mTotalLines;
	std::unordered_map<std::size_t, FormattedLine> mLines;
public:
	/**
	 * Returns the number of lines
	 */
	std::size_t numLines() const override;

	/**
	 * Sets the number of lines
	 * @param count The total number of lines
	 */
	void setNumLines(std::size_t count);

	/**
	 * Returns the formatting for the given line
	 * @param index The index
	 */
	const FormattedLine& getLine(std::size_t index) const override;

	/**
	 * Adds the given line
	 * @param index The index for the line
	 * @param tokens The tokens on the line
	 */
	void addLine(std::size_t index, FormattedLine tokens);

	/**
	 * Indicates if the given line exists
	 * @param index The index
	 */
	bool hasLine(std::size_t index) const;
};