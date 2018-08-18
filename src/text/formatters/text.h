#pragma once

#include "../textformatter.h"

/**
 * Defines text formatting rules
 */
class TextFormatterRules : public FormatterRules {
private:
	String mEmpty;
public:
	virtual FormatMode mode() const override;

	virtual bool isKeyword(const String& string) const override;

	virtual const String& lineCommentStart() const override;
	virtual const String& blockCommentStart() const override;
	virtual const String& blockCommentEnd() const override;

	virtual bool isStringDelimiter(Char current) const override;
};