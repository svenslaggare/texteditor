#include "pythonformatter.h"

PythonTextFormatterRules::PythonTextFormatterRules()
	: mKeywords { {
		  "def",
		  "import",
		  "from",
		  "if",
		  "else",
		  "while",
		  "for",
		  "case",
		  "switch",
		  "break",
		  "default",
		  "return",
		  "assert",
		  "in",
		  "elsif",
		  "try",
		  "except",
		  "as",
		  "is",
		  "not",
		  "None",
	 } } {

}

bool PythonTextFormatterRules::isKeyword(const String& string) const {
	return mKeywords.isKeyword(string);
}

const String& PythonTextFormatterRules::lineCommentStart() const {
	return mLineCommentStart;
}

const String& PythonTextFormatterRules::blockCommentStart() const {
	return mBlockCommentStart;
}

const String& PythonTextFormatterRules::blockCommentEnd() const {
	return mBlockCommentEnd;
}

bool PythonTextFormatterRules::isStringDelimiter(Char current) const {
	return current == '"' || current == '\'';
}