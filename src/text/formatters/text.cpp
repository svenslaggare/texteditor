#include "text.h"

FormatMode TextFormatterRules::mode() const {
	return FormatMode::Text;
}

bool TextFormatterRules::isKeyword(const String& string) const {
	return false;
}

const String& TextFormatterRules::lineCommentStart() const {
	return mEmpty;
}

const String& TextFormatterRules::blockCommentStart() const {
	return mEmpty;
}

const String& TextFormatterRules::blockCommentEnd() const {
	return mEmpty;
}

bool TextFormatterRules::isStringDelimiter(Char current) const {
	return false;
}
