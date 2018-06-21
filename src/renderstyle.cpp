#include "renderstyle.h"
#include "text.h"

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
