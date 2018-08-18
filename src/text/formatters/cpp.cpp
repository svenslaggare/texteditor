#include "cpp.h"

CppFormatterRules::CppFormatterRules()
	: mKeywords { {
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

		  "inline",
		  "static",

		  "struct",
		  "class",
		  "enum",
		  "namespace",

		  "public",
		  "private",

		  "auto",
		  "void",
		  "const",
		  "unsigned",
		  "char",
		  "int",
		  "short",
		  "long",
		  "float",
		  "double",
		  "bool",
		  "nullptr",

		  "#include",
		  "#if",
		  "#define",
		  "#define",
		  "#ifdef",
		  "#infdef",
		  "#endif",
		  "#else"
	 } } {

}

FormatMode CppFormatterRules::mode() const {
	return FormatMode::Code;
}

bool CppFormatterRules::isKeyword(const String& string) const {
	return mKeywords.isKeyword(string);
}

const String& CppFormatterRules::lineCommentStart() const {
	return mLineCommentStart;
}

const String& CppFormatterRules::blockCommentStart() const {
	return mBlockCommentStart;
}

const String& CppFormatterRules::blockCommentEnd() const {
	return mBlockCommentEnd;
}

bool CppFormatterRules::isStringDelimiter(Char current) const {
	return current == '"' || current == '\'';
}