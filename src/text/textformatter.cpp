#include "textformatter.h"
#include "text.h"
#include "../rendering/font.h"
#include "../rendering/textrender.h"
#include "../rendering/renderstyle.h"
#include "../rendering/renderviewport.h"
#include "../helpers.h"
#include "../external/tsl/array_set.h"

#include <iostream>
#include <unordered_set>

#include "../external/tsl/array_set.h"

LineTokens::LineTokens() {

}

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

String LineTokens::toString() const {
	String line;

	for (auto& token : tokens) {
		line += token.text;
	}

	return line;
}

namespace {
	class KeywordList {
	private:
//		std::unordered_set<String> mKeywords;
		tsl::array_set<Char> mKeywords;
		std::size_t mMaxLength = 0;
	public:
		explicit KeywordList(const std::unordered_set<std::string>& keywords) {
			for (auto& keyword : keywords) {
				auto keywordStr = Helpers::fromString<String>(keyword);
				mKeywords.insert(keywordStr);
				mMaxLength = std::max(mMaxLength, keyword.size());
			}
		}

		inline bool isKeyword(const String& str) const {
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
		"nullptr",

		"#include",
		"#if",
		"#define",
		"#define",
		"#ifdef",
		"#infdef",
		"#endif",
		"#else",
	} };

	enum class State {
		Text,
		String,
		Comment,
		BlockComment,
	};

	struct FormatterStateMachine {
		FormatMode mode;
		const Font& font;
		const RenderStyle& renderStyle;
		const RenderViewPort& viewPort;
		FormattedText& formattedText;

		std::size_t lineNumber = 0;
		std::size_t blockCommentStart = 0;

		State state = State::Text;
		bool isWhitespace = false;
		bool isEscaped = false;

		LineTokens lineTokens;
		Token currentToken;
		float currentWidth = 0.0f;

		Char prevChar = '\0';

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
				if (state == State::BlockComment) {
					lineTokens.reformatStartSearch = (std::int64_t)blockCommentStart - (std::int64_t)lineNumber;
				}

				tryMakeKeyword();
				lineTokens.addToken(std::move(currentToken));

				if (resetState) {
					currentToken = {};
				} else {
					currentToken.text = {};
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

		void addChar(Char character, float advanceX) {
			currentToken.text += character;
			currentWidth += advanceX;
			isEscaped = false;
		}

		void handleTab() {
			addChar('\t', renderStyle.getAdvanceX(font, '\t'));
		}

		void handleText(Char current, float advanceX) {
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
					if (prevChar == '/') {
						// Remove '/' from last token
 						currentToken.text.erase(currentToken.text.begin() + currentToken.text.size() - 1, currentToken.text.end());

						newToken(TokenType::Text, true);
						for (int i = 0; i < 2; i++) {
							addChar(current, advanceX);
						}

						state = State::Comment;
						currentToken.type = TokenType::Comment;
					} else {
						addChar(current, advanceX);
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
				case '&':
					newToken(TokenType::Text, true);
					addChar(current, advanceX);
					newToken(TokenType::Text, true);
					break;
				case '*':
					if (prevChar == '/') {
						// Remove '/' from last token
						currentToken.text.erase(currentToken.text.begin() + currentToken.text.size() - 1, currentToken.text.end());

						newToken(TokenType::Text, true);
						addChar(prevChar, advanceX);
						addChar(current, advanceX);
						state = State::BlockComment;
						currentToken.type = TokenType::Comment;
						blockCommentStart = lineNumber;
					} else {
						newToken(TokenType::Text, true);
						addChar(current, advanceX);
						newToken(TokenType::Text, true);
					}
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

		void handleString(Char current, float advanceX) {
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

		void handleComment(Char current, float advanceX) {
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

		void handleBlockComment(Char current, float advanceX) {
			switch (current) {
				case '\n':
					createNewLine(false, false);
					break;
				case '\t':
					handleTab();
					break;
				case '/':
					if (prevChar == '*') {
						lineTokens.reformatStartSearch = (std::int64_t)blockCommentStart - (std::int64_t)lineNumber;
						formattedText.lines()[blockCommentStart].reformatAmount = (std::int64_t)lineNumber - (std::int64_t)blockCommentStart;

						addChar(current, advanceX);
						newToken(TokenType::Text);
						state = State::Text;
					} else {
						addChar(current, advanceX);
					}
					break;
				default:
					addChar(current, advanceX);
					break;
			}
		}

		void handleCodeMode(Char current, float advanceX) {
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
				case State::BlockComment:
					handleBlockComment(current, advanceX);
					break;
			}
		}

		void handleTextMode(Char current, float advanceX) {
			if (current == '\n') {
				createNewLine();
			} else if (current == '\t') {
				handleTab();
			} else {
				addChar(current, advanceX);
			}
		}

		void process(Char current) {
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

std::vector<LineTokens>& FormattedText::lines() {
	return mLines;
}

const LineTokens& FormattedText::getLine(std::size_t index) const {
	return mLines.at(index);
}

void FormattedText::addLine(LineTokens tokens) {
	mLines.push_back(std::move(tokens));
}

std::size_t PartialFormattedText::numLines() const {
	return mTotalLines;
}

void PartialFormattedText::setNumLines(std::size_t count) {
	mTotalLines = count;
}

const LineTokens& PartialFormattedText::getLine(std::size_t index) const {
	return mLines.at(index);
}

void PartialFormattedText::addLine(std::size_t index, LineTokens tokens) {
	mLines[index] = std::move(tokens);
}

bool PartialFormattedText::hasLine(std::size_t index) const {
	return mLines.count(index) > 0;
}

TextFormatter::TextFormatter(FormatMode mode)
	: mMode(mode) {

}

void TextFormatter::formatLine(const Font& font,
							   const RenderStyle& renderStyle,
							   const RenderViewPort& viewPort,
							   const String& line,
							   LineTokens& formattedLine) {
	FormattedText formattedText;
	FormatterStateMachine stateMachine(mMode, font, renderStyle, viewPort, formattedText);

	for (auto current : line) {
		stateMachine.process(current);
	}

	stateMachine.process('\n');

	if (!stateMachine.lineTokens.tokens.empty()) {
		stateMachine.createNewLine();
	}

	formattedLine = formattedText.getLine(0);
}

void TextFormatter::formatLines(const Font& font,
								const RenderStyle& renderStyle,
								const RenderViewPort& viewPort,
								const std::vector<const String*>& lines,
								std::vector<LineTokens>& formattedLines) {
	FormattedText formattedText;
	FormatterStateMachine stateMachine(mMode, font, renderStyle, viewPort, formattedText);

	for (auto& line : lines) {
		for (auto current : *line) {
			stateMachine.process(current);
		}

		stateMachine.process('\n');

		if (!stateMachine.lineTokens.tokens.empty()) {
			stateMachine.createNewLine();
		}
	}

	formattedLines = std::move(formattedText.lines());
}

void TextFormatter::format(const Font& font,
						   const RenderStyle& renderStyle,
						   const RenderViewPort& viewPort,
						   const Text& text,
						   FormattedText& formattedText) {
	FormatterStateMachine stateMachine(mMode, font, renderStyle, viewPort, formattedText);

	text.forEachLine([&](const String& line) {
		for (auto& current : line) {
			stateMachine.process(current);
		}

		stateMachine.process('\n');
	});

	if (!stateMachine.lineTokens.tokens.empty()) {
		stateMachine.createNewLine();
	}
}
