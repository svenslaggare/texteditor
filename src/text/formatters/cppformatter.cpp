#include "cppformatter.h"

CppTextFormatterRules::CppTextFormatterRules()
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

bool CppTextFormatterRules::isKeyword(const String& string) const {
	return mKeywords.isKeyword(string);
}

const String& CppTextFormatterRules::lineCommentStart() const {
	return mLineCommentStart;
}

const String& CppTextFormatterRules::blockCommentStart() const {
	return mBlockCommentStart;
}

const String& CppTextFormatterRules::blockCommentEnd() const {
	return mBlockCommentEnd;
}
