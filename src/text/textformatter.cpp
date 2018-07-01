#include "textformatter.h"
#include "../rendering/font.h"
#include "../rendering/textrender.h"
#include "../rendering/renderstyle.h"
#include "../rendering/renderviewport.h"
#include "text.h"
#include "../helpers.h"

#include <iostream>
#include <unordered_set>

void LineTokens::addToken(Token token) {
	tokens.push_back(std::move(token));
}

std::size_t LineTokens::length() const {
	std::size_t count = 0;

	for (auto& token : tokens) {
		count += token.text.size();
	}

	return count;
}

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

		std::size_t lineNumber = 0;

		State state = State::Text;
		bool isWhitespace = false;
		bool isEscaped = false;

		LineTokens lineTokens;
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

		void createNewLine(bool resetState = true, bool continueWithLine = false) {
			if (mode == FormatMode::Code) {
				tryMakeKeyword();
				lineTokens.addToken(std::move(currentToken));

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
				lineTokens.addToken(std::move(currentToken));
				currentToken = {};
				currentWidth = 0.0f;
			}

			lineTokens.number = lineNumber;
			std::size_t offsetFromTextLine = 0;
			if (!continueWithLine) {
				lineNumber++;
			} else {
				offsetFromTextLine = lineTokens.offsetFromTextLine + lineTokens.length();
			}

			formattedText.addLine(std::move(lineTokens));
			lineTokens = {};
			lineTokens.offsetFromTextLine = offsetFromTextLine;
			lineTokens.isContinuation = continueWithLine;
		}

		void newToken(TokenType type = TokenType::Text, bool makeKeyword = false) {
			if (makeKeyword) {
				tryMakeKeyword();
			}

			lineTokens.addToken(std::move(currentToken));
			currentToken = {};
			currentToken.type = type;
		}

		void addChar(char c, float advanceX) {
			currentToken.text += c;
			currentWidth += advanceX;
			isEscaped = false;
		}

		void handleTab() {
			addChar('\t', renderStyle.getAdvanceX(font, '\t'));
		}

		void handleText(char current, float advanceX) {
			switch (current) {
				case '\n':
					createNewLine();
					break;
				case '\t':
					newToken(TokenType::Text, true);
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
					newToken(TokenType::Text, true);
					addChar(current, advanceX);
					isWhitespace = true;
					break;
				case ',':
				case '(':
				case ')':
				case '*':
					newToken(TokenType::Text, true);
					addChar(current, advanceX);
					newToken(TokenType::Text, true);
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
			auto advanceX = renderStyle.getAdvanceX(font, current);

			if (renderStyle.wordWrap) {
				if (currentWidth + advanceX > viewPort.width) {
					createNewLine(false, true);
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

const LineTokens& FormattedText::getLine(std::size_t index) const {
	return mLines.at(index);
}

void FormattedText::addLine(LineTokens tokens) {
	mLines.push_back(std::move(tokens));
}

TextFormatter::TextFormatter(FormatMode mode)
	: mMode(mode) {

}

void TextFormatter::format(const Font& font,
						   const RenderStyle& renderStyle,
						   const RenderViewPort& viewPort,
						   const Text& text,
						   FormattedText& formattedText) {
	FormatterStateMachine stateMachine(mMode, font, renderStyle, viewPort, formattedText);

	text.forEach([&](std::size_t i, char current) {
		stateMachine.process(current);
	});

	if (!stateMachine.lineTokens.tokens.empty()) {
		stateMachine.createNewLine();
	}
}
