#include "textformatter.h"
#include "../rendering/font.h"
#include "../rendering/textrender.h"
#include "../rendering/renderstyle.h"
#include "../rendering/renderviewport.h"
#include "text.h"
#include "../helpers.h"

#include <iostream>
#include <unordered_set>

namespace {
	class KeywordList {
	private:
		std::unordered_set<std::string> mKeywords;
		std::size_t mMaxLength = 0;
	public:
		explicit KeywordList(std::unordered_set<std::string> keywords)
			: mKeywords(std::move(keywords)) {
			for (auto& keyword : mKeywords) {
				mMaxLength = std::max(mMaxLength, keyword.size());
			}
		}

		inline bool isKeyword(const std::string& str) const {
			if (str.size() <= mMaxLength) {
				return mKeywords.count(str) > 0;
			}

			return false;
		}
	};

	const KeywordList keywords { {
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

		"#include",
		"#if",
		"#define",
		"#define",
		"#ifdef",
		"#endif",
		"#else",
	} };

	enum class State {
		Text,
		String,
		Comment
	};

	struct FormatterStateMachine {
		FormatMode mode;
		const Font& font;
		const RenderStyle& renderStyle;
		const RenderViewPort& viewPort;
		FormattedText& formattedText;

		State state = State::Text;
		bool isWhitespace = false;
		bool isEscaped = false;

		std::vector<Token> tokens;
		Token currentToken;
		float currentWidth = 0.0f;

		char prevChar = '\0';

		FormatterStateMachine(FormatMode mode,
							  const Font& font,
							  const RenderStyle& renderStyle,
							  const RenderViewPort& viewPort,
							  FormattedText& formattedText)
			: mode(mode),
			  font(font),
			  renderStyle(renderStyle),
			  viewPort(viewPort),
			  formattedText(formattedText) {

		}

		void tryMakeKeyword() {
			if (keywords.isKeyword(currentToken.text)) {
				currentToken.type = TokenType::Keyword;
			}
		}

		void createNewLine(bool resetState = true) {
			if (mode == FormatMode::Code) {
				tryMakeKeyword();
				tokens.push_back(std::move(currentToken));
				formattedText.addLine(std::move(tokens));
				tokens = {};

				if (resetState) {
					currentToken = {};
				} else {
					currentToken.text = "";
				}

				currentWidth = 0.0f;

				if (resetState) {
					isWhitespace = false;
					isEscaped = false;
					state = State::Text;
				}
			} else {
				tokens.push_back(std::move(currentToken));
				formattedText.addLine(std::move(tokens));
				tokens = {};
				currentToken = {};
				currentWidth = 0.0f;
			}
		}

		void newToken(TokenType type = TokenType::Text) {
			tryMakeKeyword();
			tokens.push_back(std::move(currentToken));
			currentToken = {};
			currentToken.type = type;
		}

		void addChar(char c, float advanceX) {
			currentToken.text += c;
			currentWidth += advanceX;
			isEscaped = false;
		}

		void handleTab() {
			auto advanceX = font[' '].advanceX;
			for (std::size_t i = 0; i < renderStyle.spacesPerTab; i++) {
				addChar(' ', advanceX);
			}
		}

		void handleText(char current, float advanceX) {
			switch (current) {
				case '\n':
					createNewLine();
					break;
				case '\t':
					newToken();
					handleTab();
					isWhitespace = true;
					break;
				case '"':
					newToken(TokenType::String);
					state = State::String;
					addChar(current, advanceX);
					break;
				case '/':
					addChar(current, advanceX);
					if (prevChar == '/') {
						state = State::Comment;
						currentToken.type = TokenType::Comment;
					}
					break;
				case ' ':
					newToken();
					addChar(current, advanceX);
					isWhitespace = true;
					break;
				case ',':
				case '(':
				case ')':
				case '*':
					newToken();
					addChar(current, advanceX);
					newToken();
					break;
				default:
					if (isWhitespace) {
						newToken();
						isWhitespace = false;
					}

					addChar(current, advanceX);
					break;
			}
		}

		void handleString(char current, float advanceX) {
			switch (current) {
				case '\n':
					createNewLine();
					break;
				case '\t':
					handleTab();
					break;
				case '"':
					if (!isEscaped) {
						state = State::Text;
						addChar(current, advanceX);
						newToken();
					} else {
						addChar(current, advanceX);
					}

					break;
				case '\\':
					addChar(current, advanceX);
					isEscaped = true;
					break;
				default:
					addChar(current, advanceX);
					break;
			}
		}

		void handleComment(char current, float advanceX) {
			switch (current) {
				case '\n':
					createNewLine();
					break;
				case '\t':
					handleTab();
					break;
				default:
					addChar(current, advanceX);
					break;
			}
		}

		void handleCodeMode(char current, float advanceX) {
			switch (state) {
				case State::Text:
					handleText(current, advanceX);
					break;
				case State::String:
					handleString(current, advanceX);
					break;
				case State::Comment:
					handleComment(current, advanceX);
					break;
			}
		}

		void handleTextMode(char current, float advanceX) {
			if (current == '\n') {
				createNewLine();
			} else if (current == '\t') {
				handleTab();
			} else {
				addChar(current, advanceX);
			}
		}

		void process(char current) {
			auto advanceX = font[current].advanceX;

			if (renderStyle.wordWrap) {
				if (currentWidth + advanceX > viewPort.width) {
					createNewLine(false);
				}
			}

			switch (mode) {
				case FormatMode::Text:
					handleTextMode(current, advanceX);
					break;
				case FormatMode::Code:
					handleCodeMode(current, advanceX);
					break;
			}

			prevChar = current;
		}
	};
}

std::size_t FormattedText::numLines() const {
	return mLines.size();
}

const std::vector<Token>& FormattedText::getLine(std::size_t index) const {
	return mLines[index];
}

void FormattedText::addLine(std::vector<Token> tokens) {
	mLines.push_back(std::move(tokens));
}

TextFormatter::TextFormatter(FormatMode mode)
	: mMode(mode) {

}

void TextFormatter::format(const Font& font,
						   const RenderStyle& renderStyle,
						   const RenderViewPort& viewPort,
						   const std::string& text,
						   FormattedText& formattedText) {
	FormatterStateMachine stateMachine(mMode, font, renderStyle, viewPort, formattedText);

	for (auto& current : text) {
		stateMachine.process(current);
	}

	stateMachine.createNewLine();
}
