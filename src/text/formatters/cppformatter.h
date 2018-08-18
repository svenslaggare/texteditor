#pragma once
#include "../textformatter.h"

/**
 * Defines C++ formatting rules
 */
class CppTextFormatterRules : public TextFormatterRules {
private:
	KeywordList mKeywords;
	String mLineCommentStart = u"//";
	String mBlockCommentStart = u"/*";
	String mBlockCommentEnd = u"*/";
public:
	CppTextFormatterRules();
	virtual ~CppTextFormatterRules() override = default;

	virtual bool isKeyword(const String& string) const override;

	virtual const String& lineCommentStart() const override;
	virtual const String& blockCommentStart() const override;
	virtual const String& blockCommentEnd() const override;
	virtual bool isStringDelimiter(Char current) const override;
};