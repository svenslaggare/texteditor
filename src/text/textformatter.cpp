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
#include "formatters/cppformatter.h"

FormatterStateMachine::FormatterStateMachine(FormatMode mode,
											 const TextFormatterRules& textFormatterRules,
											 const Font& font,
											 const RenderStyle& renderStyle,
											 const RenderViewPort& viewPort,
											 FormattedLines& formattedLines)
	: mMode(mode),
	  mTextFormatterRules(textFormatterRules),
	  mFont(font),
	  mRenderStyle(renderStyle),
	  mViewPort(viewPort),
	  mFormattedLines(formattedLines),
	  mPrevCharBuffer(5) {

}

State FormatterStateMachine::state() const {
	return mState;
}

const FormattedLine& FormatterStateMachine::currentFormattedLine() const {
	return mCurrentFormattedLine;
}

void FormatterStateMachine::removeChars(std::size_t count) {
	std::size_t toRemoveLeft = count;
	Token* currentToken = &mCurrentToken;
	auto currentTokenIterator = mCurrentFormattedLine.tokens.end();

	while (toRemoveLeft > 0) {
		auto thisRemoved = std::min(toRemoveLeft, currentToken->text.size());

		if (thisRemoved > 0) {
			currentToken->text.erase(
				currentToken->text.begin() + currentToken->text.size() - thisRemoved,
				currentToken->text.end());
		}

		toRemoveLeft -= thisRemoved;
		if (toRemoveLeft > 0) {
			if (currentTokenIterator == mCurrentFormattedLine.tokens.begin()) {
				break;
			}

			if (currentTokenIterator == mCurrentFormattedLine.tokens.end()) {
				currentTokenIterator = mCurrentFormattedLine.tokens.end() - 1;
			} else {
				currentTokenIterator--;
			}

			currentToken = &(*currentTokenIterator);
		}
	}
}

bool FormatterStateMachine::isPrevCharsMatch(const String& string, Char current, String& prevChars) {
	if (current == string.back()) {
		prevChars = getPrevChars(string.size() - 1);
		if (prevChars == string.substr(0, string.size() - 1)) {
			return true;
		}
	}

	return false;
}

void FormatterStateMachine::tryMakeKeyword() {
	if (mTextFormatterRules.isKeyword(mCurrentToken.text)) {
		mCurrentToken.type = TokenType::Keyword;
	}
}

void FormatterStateMachine::createNewLine(bool resetState, bool continueWithLine, bool allowKeyword) {
	if (mMode == FormatMode::Code) {
		if (mState == State::BlockComment) {
			mCurrentFormattedLine.reformatStartSearch = (std::int64_t)mBlockCommentStartIndex - (std::int64_t)mLineNumber;
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

String FormatterStateMachine::getPrevChars(std::size_t size) {
	String prevChars;
	mPrevCharBuffer.forEachElement([&](auto& value) {
		prevChars += value;
	}, size);

	return prevChars;
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
		default: {
			String prevChars;
			if (isPrevCharsMatch(mTextFormatterRules.lineCommentStart(), current, prevChars)) {
				// Remove prevChars from last token
				removeChars(prevChars.size());

				newToken(TokenType::Text, true);
				for (auto& prevCurrent : prevChars) {
					addChar(prevCurrent, mRenderStyle.getAdvanceX(mFont, prevCurrent));
				}
				addChar(current, advanceX);

				mState = State::Comment;
				mCurrentToken.type = TokenType::Comment;
				break;
			}

			if (isPrevCharsMatch(mTextFormatterRules.blockCommentStart(), current, prevChars)) {
				// Remove prevChars from last token
				removeChars(prevChars.size());

				newToken(TokenType::Text, true);
				for (auto& prevCurrent : prevChars) {
					addChar(prevCurrent, mRenderStyle.getAdvanceX(mFont, prevCurrent));
				}
				addChar(current, advanceX);

				mState = State::BlockComment;
				mCurrentToken.type = TokenType::Comment;
				mCurrentFormattedLine.mayRequireSearch = true;
				mBlockCommentStartIndex = mLineNumber;

				break;
			} else if (current == mTextFormatterRules.blockCommentStart().back()) {
				newToken(TokenType::Text, true);
				addChar(current, advanceX);
				newToken(TokenType::Text, true);

				break;
			}

			if (mIsWhitespace) {
				newToken();
				mIsWhitespace = false;
			}

			addChar(current, advanceX);
			break;
		}
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
			mFormattedLines[mBlockCommentStartIndex].reformatAmount = (std::int64_t)mLineNumber - (std::int64_t)mBlockCommentStartIndex;
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
		default:
			String prevChars;
			if (isPrevCharsMatch(mTextFormatterRules.blockCommentEnd(), current, prevChars)) {
				updateStartFormatInformation();
				mCurrentFormattedLine.reformatStartSearch = (std::int64_t)mBlockCommentStartIndex - (std::int64_t)mLineNumber;
				mCurrentFormattedLine.mayRequireSearch = true;

				addChar(current, advanceX);
				newToken(TokenType::Text);
				mState = State::Text;
				break;
			}

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

	mPrevCharBuffer.add(current);
}

TextFormatter::TextFormatter(FormatMode mode)
	: mMode(mode),
	  mRules(std::make_unique<CppTextFormatterRules>()) {

}

const TextFormatterRules& TextFormatter::rules() const {
	return *mRules;
}

void TextFormatter::formatLine(const Font& font,
							   const RenderStyle& renderStyle,
							   const RenderViewPort& viewPort,
							   const String& line,
							   FormattedLine& formattedLine) {
	FormattedLines formattedLines;
	FormatterStateMachine stateMachine(mMode, *mRules, font, renderStyle, viewPort, formattedLines);

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
	FormatterStateMachine stateMachine(mMode, *mRules, font, renderStyle, viewPort, formattedLines);

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
	FormatterStateMachine stateMachine(mMode, *mRules, font, renderStyle, viewPort, formattedLines);

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