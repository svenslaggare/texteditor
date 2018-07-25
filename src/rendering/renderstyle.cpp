#include "renderstyle.h"
#include "../text/text.h"
#include "../text/textformatter.h"
#include "font.h"

glm::vec3 RenderStyle::getColor(const Token& token) const {
	switch (token.type) {
		case TokenType::Keyword:
			return keywordColor;
		case TokenType::String:
			return stringColor;
		case TokenType::Text:
			return textColor;
		case TokenType::Comment:
			return commentColor;
	}
}

float RenderStyle::getAdvanceX(const Font& font, Char character) const {
	auto advanceX = font.getAdvanceX(character);

	if (character == '\t') {
		return advanceX * spacesPerTab;
	}

	return advanceX;
}
