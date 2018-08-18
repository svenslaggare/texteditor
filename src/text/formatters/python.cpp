#include "python.h"

PythonFormatterRules::PythonFormatterRules()
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

FormatMode PythonFormatterRules::mode() const {
	return FormatMode::Code;
}

bool PythonFormatterRules::isKeyword(const String& string) const {
	return mKeywords.isKeyword(string);
}

const String& PythonFormatterRules::lineCommentStart() const {
	return mLineCommentStart;
}

const String& PythonFormatterRules::blockCommentStart() const {
	return mBlockCommentStart;
}

const String& PythonFormatterRules::blockCommentEnd() const {
	return mBlockCommentEnd;
}

bool PythonFormatterRules::isStringDelimiter(Char current) const {
	return current == '"' || current == '\'';
}