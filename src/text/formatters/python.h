#pragma once
#include "../textformatter.h"

/**
 * Defines Python formatting rules
 */
class PythonFormatterRules : public FormatterRules {
private:
	KeywordList mKeywords;
	String mLineCommentStart = u"#";
	String mBlockCommentStart = u"\"\"\"";
	String mBlockCommentEnd = u"\"\"\"";
public:
	PythonFormatterRules();
	virtual ~PythonFormatterRules() override = default;

	inline virtual FormatMode mode() const override {
		return FormatMode::Code;
	}

	inline virtual bool isKeyword(const String& string) const override {
		return mKeywords.isKeyword(string);
	}

	inline virtual const String& lineCommentStart() const override {
		return mLineCommentStart;
	}

	inline virtual const String& blockCommentStart() const override {
		return mBlockCommentStart;
	}

	inline virtual const String& blockCommentEnd() const override {
		return mBlockCommentEnd;
	}

	inline virtual bool isStringDelimiter(Char current) const override {
		return current == '"' || current == '\'';
	}
};