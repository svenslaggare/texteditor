#include "textformatter.h"
#include "font.h"
#include "textrender.h"
#include "renderstyle.h"
#include "renderviewport.h"
#include "text.h"
#include "helpers.h"

#include <iostream>
#include <unordered_set>

namespace {
	const std::unordered_set<std::string> keywords = {
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
	};

	enum class State {
		Text,
		String,
		Comment
	};

	struct FormatterStateMachine {
		FormatMode mode;
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

		FormatterStateMachine(FormatMode mode, const RenderStyle& renderStyle, const RenderViewPort& viewPort, FormattedText& formattedText)
			: mode(mode), renderStyle(renderStyle), viewPort(viewPort), formattedText(formattedText) {

		}

		void tryMakeKeyword() {
			if (keywords.count(currentToken.text)) {
				currentToken.type = TokenType::Keyword;
			}
		}

		void createNewLine(bool resetState = true) {
			tryMakeKeyword();
			tokens.push_back(std::move(currentToken));
			formattedText.lines.push_back(std::move(tokens));

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

		void handleText(char current, float advanceX) {
			switch (current) {
				case '\n':
					createNewLine();
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
				default:
					addChar(current, advanceX);
					break;
			}
		}

		void handleTextMode(char current, float advanceX) {
			if (current == '\n') {
				createNewLine();
			} else {
				addChar(current, advanceX);
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

		void process(char current, float advanceX) {
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

TextFormatter::TextFormatter(FormatMode mode)
	: mMode(mode) {

}

void TextFormatter::format(const Font& font,
						   const RenderStyle& renderStyle,
						   const RenderViewPort& viewPort,
						   const std::string& text,
						   FormattedText& formattedText) {
	auto processedText = text;
	processedText = Helpers::replaceAll(processedText, "\t", std::string((std::size_t)renderStyle.spacesPerTab, ' '));

	FormatterStateMachine stateMachine(mMode, renderStyle, viewPort, formattedText);

	for (auto& current : processedText) {
		stateMachine.process(current, font[current].advanceX);
	}

	stateMachine.createNewLine();
}
