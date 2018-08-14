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
}

FormatterStateMachine::FormatterStateMachine(FormatMode mode,
											 const Font& font,
											 const RenderStyle& renderStyle,
											 const RenderViewPort& viewPort,
											 FormattedLines& formattedLines)
	: mMode(mode),
	  mFont(font),
	  mRenderStyle(renderStyle),
	  mViewPort(viewPort),
	  mFormattedLines(formattedLines) {

}

State FormatterStateMachine::state() const {
	return mState;
}

const FormattedLine& FormatterStateMachine::currentFormattedLine() const {
	return mCurrentFormattedLine;
}

void FormatterStateMachine::tryMakeKeyword() {
	if (keywords.isKeyword(mCurrentToken.text)) {
		mCurrentToken.type = TokenType::Keyword;
	}
}

void FormatterStateMachine::createNewLine(bool resetState, bool continueWithLine, bool allowKeyword) {
	if (mMode == FormatMode::Code) {
		if (mState == State::BlockComment) {
			mCurrentFormattedLine.reformatStartSearch = (std::int64_t)mBlockCommentStart - (std::int64_t)mLineNumber;
		}

		if (allowKeyword) {
			tryMakeKeyword();
		}

		mCurrentFormattedLine.addToken(std::move(mCurrentToken));

		if (resetState) {
			mCurrentToken = {};
		} else {
			mCurrentToken.text = {};
		}

		mCurrentWidth = 0.0f;

		if (resetState) {
			mIsWhitespace = false;
			mIsEscaped = false;
			mState = State::Text;
		}
	} else {
		mCurrentFormattedLine.addToken(std::move(mCurrentToken));
		mCurrentToken = {};
		mCurrentWidth = 0.0f;
	}

	mCurrentFormattedLine.number = mLineNumber;
	std::size_t offsetFromTextLine = 0;
	if (!continueWithLine) {
		mLineNumber++;
	} else {
		offsetFromTextLine = mCurrentFormattedLine.offsetFromTextLine + mCurrentFormattedLine.length();
	}

	mFormattedLines.push_back(std::move(mCurrentFormattedLine));
	mCurrentFormattedLine = {};
	mCurrentFormattedLine.offsetFromTextLine = offsetFromTextLine;
	mCurrentFormattedLine.isContinuation = continueWithLine;
}

void FormatterStateMachine::newToken(TokenType type, bool makeKeyword) {
	if (makeKeyword) {
		tryMakeKeyword();
	}

	mCurrentFormattedLine.addToken(std::move(mCurrentToken));
	mCurrentToken = {};
	mCurrentToken.type = type;
}

void FormatterStateMachine::addChar(Char character, float advanceX) {
	mCurrentToken.text += character;
	mCurrentWidth += advanceX;
	mIsEscaped = false;
}

void FormatterStateMachine::handleTab() {
	addChar('\t', mRenderStyle.getAdvanceX(mFont, '\t'));
}

void FormatterStateMachine::handleText(Char current, float advanceX) {
	switch (current) {
		case '\n':
			createNewLine();
			break;
		case '\t':
			newToken(TokenType::Text, true);
			handleTab();
			mIsWhitespace = true;
			break;
		case '"':
			newToken(TokenType::String);
			mState = State::String;
			addChar(current, advanceX);
			break;
		case '/':
			if (mPrevChar == '/') {
				// Remove '/' from last token
				mCurrentToken.text.erase(mCurrentToken.text.begin() + mCurrentToken.text.size() - 1, mCurrentToken.text.end());

				newToken(TokenType::Text, true);
				for (int i = 0; i < 2; i++) {
					addChar(current, advanceX);
				}

				mState = State::Comment;
				mCurrentToken.type = TokenType::Comment;
			} else {
				addChar(current, advanceX);
			}
			break;
		case ' ':
			newToken(TokenType::Text, true);
			addChar(current, advanceX);
			mIsWhitespace = true;
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
			if (mPrevChar == '/') {
				// Remove '/' from last token
				mCurrentToken.text.erase(mCurrentToken.text.begin() + mCurrentToken.text.size() - 1, mCurrentToken.text.end());

				newToken(TokenType::Text, true);
				addChar(mPrevChar, advanceX);
				addChar(current, advanceX);
				mState = State::BlockComment;
				mCurrentToken.type = TokenType::Comment;
				mCurrentFormattedLine.mayRequireSearch = true;
				mBlockCommentStart = mLineNumber;
			} else {
				newToken(TokenType::Text, true);
				addChar(current, advanceX);
				newToken(TokenType::Text, true);
			}
			break;
		default:
			if (mIsWhitespace) {
				newToken();
				mIsWhitespace = false;
			}

			addChar(current, advanceX);
			break;
	}
}

void FormatterStateMachine::handleString(Char current, float advanceX) {
	switch (current) {
		case '\n':
			createNewLine();
			break;
		case '\t':
			handleTab();
			break;
		case '"':
			if (!mIsEscaped) {
				mState = State::Text;
				addChar(current, advanceX);
				newToken();
			} else {
				addChar(current, advanceX);
			}

			break;
		case '\\':
			addChar(current, advanceX);
			mIsEscaped = true;
			break;
		default:
			addChar(current, advanceX);
			break;
	}
}

void FormatterStateMachine::handleComment(Char current, float advanceX) {
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

void FormatterStateMachine::handleBlockComment(Char current, float advanceX) {
	auto updateStartFormatInformation = [&]() {
		if (!mFormattedLines.empty()) {
			mFormattedLines[mBlockCommentStart].reformatAmount =	(std::int64_t) mLineNumber - (std::int64_t) mBlockCommentStart;
		}
	};

	switch (current) {
		case '\n':
			updateStartFormatInformation();
			createNewLine(false, false, false);
			break;
		case '\t':
			handleTab();
			break;
		case '/':
			if (mPrevChar == '*') {
				updateStartFormatInformation();
				mCurrentFormattedLine.reformatStartSearch = (std::int64_t)mBlockCommentStart - (std::int64_t)mLineNumber;
				mCurrentFormattedLine.mayRequireSearch = true;

				addChar(current, advanceX);
				newToken(TokenType::Text);
				mState = State::Text;
			} else {
				addChar(current, advanceX);
			}
			break;
		default:
			addChar(current, advanceX);
			break;
	}
}

void FormatterStateMachine::handleCodeMode(Char current, float advanceX) {
	switch (mState) {
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

void FormatterStateMachine::handleTextMode(Char current, float advanceX) {
	if (current == '\n') {
		createNewLine();
	} else if (current == '\t') {
		handleTab();
	} else {
		addChar(current, advanceX);
	}
}

void FormatterStateMachine::process(Char current) {
	auto advanceX = mRenderStyle.getAdvanceX(mFont, current);

	if (mRenderStyle.wordWrap) {
		if (mCurrentWidth + advanceX > mViewPort.width) {
			createNewLine(false, true);
		}
	}

	switch (mMode) {
		case FormatMode::Text:
			handleTextMode(current, advanceX);
			break;
		case FormatMode::Code:
			handleCodeMode(current, advanceX);
			break;
	}

	mPrevChar = current;
}

TextFormatter::TextFormatter(FormatMode mode)
	: mMode(mode) {

}

void TextFormatter::formatLine(const Font& font,
							   const RenderStyle& renderStyle,
							   const RenderViewPort& viewPort,
							   const String& line,
							   FormattedLine& formattedLine) {
	FormattedLines formattedLines;
	FormatterStateMachine stateMachine(mMode, font, renderStyle, viewPort, formattedLines);

	for (auto current : line) {
		stateMachine.process(current);
	}

	stateMachine.process('\n');

	if (!stateMachine.currentFormattedLine().tokens.empty()) {
		stateMachine.createNewLine();
	}

	formattedLine = std::move(formattedLines.front());
}

void TextFormatter::formatLines(const Font& font,
								const RenderStyle& renderStyle,
								const RenderViewPort& viewPort,
								const std::vector<const String*>& lines,
								FormattedLines& formattedLines) {
	FormatterStateMachine stateMachine(mMode, font, renderStyle, viewPort, formattedLines);

	for (auto& line : lines) {
		for (auto current : *line) {
			stateMachine.process(current);
		}

		stateMachine.process('\n');
	}

	if (!stateMachine.currentFormattedLine().tokens.empty()) {
		stateMachine.createNewLine();
	}
}

void TextFormatter::format(const Font& font,
						   const RenderStyle& renderStyle,
						   const RenderViewPort& viewPort,
						   const Text& text,
						   FormattedLines& formattedLines) {
	FormatterStateMachine stateMachine(mMode, font, renderStyle, viewPort, formattedLines);

	text.forEachLine([&](const String& line) {
		for (auto& current : line) {
			stateMachine.process(current);
		}

		stateMachine.process('\n');
	});

	if (!stateMachine.currentFormattedLine().tokens.empty()) {
		stateMachine.createNewLine();
	}
}