#include "textrender.h"
#include "font.h"
#include "glhelpers.h"
#include "../text/textformatter.h"
#include "renderstyle.h"
#include "renderviewport.h"
#include "../interface/textview.h"
#include "../text/text.h"
#include "../interface/inputmanager.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <codecvt>
#include <locale>

#include <glm/vec3.hpp>

namespace {
	const std::size_t NUM_TRIANGLES = 6;
	const std::size_t MAX_CHARACTERS = 200;
	const std::size_t FLOATS_PER_CHARACTER = NUM_TRIANGLES * (2 + 2 + 3);

	Char convertFromChar(char character) {
		return (Char)character;
	}

	template<typename TString, typename TValue>
	TString numericToString(TValue value) {
		std::wstring_convert<std::codecvt_utf8<Char>, Char> cv;
		return cv.from_bytes(std::to_string(value));
	}

	template<typename TValue>
	std::string numericToString(TValue value) {
		return std::to_string(value);
	}
}

TextRender::TextRender(GLuint shaderProgram)
	: mShaderProgram(shaderProgram) {
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * FLOATS_PER_CHARACTER * MAX_CHARACTERS, nullptr, GL_DYNAMIC_DRAW);

	auto vertexSize = (2 + 2 + 3) * sizeof(GLfloat);
	auto posAttribute = glGetAttribLocation(shaderProgram, "vertexPosition");
	glEnableVertexAttribArray(posAttribute);
	glVertexAttribPointer(posAttribute, 2, GL_FLOAT, GL_FALSE, vertexSize, nullptr);

	auto texAttribute = glGetAttribLocation(shaderProgram, "vertexTexcoord");
	glEnableVertexAttribArray(texAttribute);
	glVertexAttribPointer(texAttribute, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)(2 * sizeof(GLfloat)));

	auto colorAttribute = glGetAttribLocation(shaderProgram, "vertexColor");
	glEnableVertexAttribArray(colorAttribute);
	glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)((2 + 2) * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

TextRender::~TextRender() {
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

void TextRender::setCharacterVertices(GLfloat* charactersVertices,
									  std::size_t offset,
									  const Font& font,
									  Char character,
									  float currentX,
									  float currentY,
									  glm::vec3 color) {
	auto& fontCharacter = font[character];

	GLfloat drawPosX = currentX + fontCharacter.bearing.x;
	GLfloat drawPosY = -currentY - (fontCharacter.size.y - fontCharacter.bearing.y);

	GLfloat drawWidth = fontCharacter.size.x;
	GLfloat drawHeight = fontCharacter.size.y;

	float top;
	float left;
	float bottom;
	float right;
	font.getTextureCoordinates(character, top, left, bottom, right);

	// Batch draw calls
	GLfloat vertices[] = {
		drawPosX, (drawPosY + drawHeight), left, top, color.r, color.g, color.b,
		drawPosX, drawPosY, left, bottom, color.r, color.g, color.b,
		(drawPosX + drawWidth), drawPosY, right, bottom, color.r, color.g, color.b,

		drawPosX, (drawPosY + drawHeight), left, top, color.r, color.g, color.b,
		(drawPosX + drawWidth), drawPosY, right, bottom, color.r, color.g, color.b,
		(drawPosX + drawWidth), (drawPosY + drawHeight), right, top, color.r, color.g, color.b
	};

	for (std::size_t i = 0; i < FLOATS_PER_CHARACTER; i++) {
		charactersVertices[offset + i] = vertices[i];
	}
}

void TextRender::setupRendering(const Font& font) {
	glBindVertexArray(mVAO);
	glUseProgram(mShaderProgram);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, font.textureMap());
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
}

void TextRender::renderView(const Font& font,
							std::size_t maxNumLines,
							const RenderViewPort& viewPort,
							glm::vec2 position,
							RenderLine renderLine) {
	setupRendering(font);

	GLfloat charactersVertices[sizeof(GLfloat) * FLOATS_PER_CHARACTER * MAX_CHARACTERS];
	std::size_t numCharactersDrawn = 0;
	std::size_t charactersOffset = 0;

	auto drawCharacters = [&]() {
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * FLOATS_PER_CHARACTER * numCharactersDrawn, charactersVertices);
		glDrawArrays(GL_TRIANGLES, 0, NUM_TRIANGLES * numCharactersDrawn);
		numCharactersDrawn = 0;
		charactersOffset = 0;
	};

	auto drawCharacter = [&]() {
		numCharactersDrawn++;
		charactersOffset += FLOATS_PER_CHARACTER;
		if (numCharactersDrawn >= MAX_CHARACTERS) {
			drawCharacters();
		}
	};

	auto cursorLineIndex = (std::int64_t)std::floor(-position.y / font.lineHeight());
	float offsetY = 0.0f;

	for (std::int64_t lineIndex = cursorLineIndex; lineIndex < (std::int64_t)maxNumLines; lineIndex++) {
		if (lineIndex >= 0) {
			glm::vec2 drawPosition(position.x, position.y + (lineIndex + 1) * font.lineHeight() + offsetY);
			auto originalY = drawPosition.y;

			RenderViewPort currentView { drawPosition, viewPort.width, viewPort.height };

			if (drawPosition.y >= viewPort.top()) {
				renderLine(
					charactersVertices,
					charactersOffset,
					drawCharacter,
					lineIndex,
					currentView,
					drawPosition);

				offsetY += drawPosition.y - originalY;
			}

			if (drawPosition.y > viewPort.bottom()) {
				break;
			}
		}
	}

	if (numCharactersDrawn > 0) {
		drawCharacters();
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TextRender::render(const Font& font,
						const RenderStyle& renderStyle,
						const RenderViewPort& viewPort,
						const BaseFormattedText& text,
						glm::vec2 position,
						float lineNumberSpacing) {
	renderView(font, text.numLines(), viewPort, position, [&](GLfloat* vertices,
															  std::size_t& offset,
															  std::function<void()> drawCharacter,
															  std::int64_t lineIndex,
															  const RenderViewPort& currentView,
															  glm::vec2& drawPosition) {
		auto& line = text.getLine((std::size_t)lineIndex);

		// // Start by drawing the line number
		// float currentLineNumberSpacing = 0.0f;
		// if (!line.isContinuation) {
		// 	auto lineNumber = numericToString<String>(line.number + 1);
		// 	for (auto& character : lineNumber) {
		// 		setCharacterVertices(
		// 			vertices,
		// 			offset,
		// 			font,
		// 			character,
		// 			drawPosition.x,
		// 			drawPosition.y,
		// 			renderStyle.lineNumberColor);

		// 		auto advanceX = renderStyle.getAdvanceX(font, character);
		// 		drawPosition.x += advanceX;
		// 		currentLineNumberSpacing += advanceX;
		// 		drawCharacter();
		// 	}
		// }

		// // Then the text on the line
		// auto actualLineNumberSpacing = lineNumberSpacing - currentLineNumberSpacing;
		// drawPosition.x += actualLineNumberSpacing;

		for (auto& token : line.tokens) {
			auto color = renderStyle.getColor(token);

			for (auto& character : token.text) {
				auto advanceX = renderStyle.getAdvanceX(font, character);

				setCharacterVertices(
					vertices,
					offset,
					font,
					character,
					drawPosition.x,
					drawPosition.y,
					color);

				drawPosition.x += advanceX;
				drawCharacter();
			}
		}
	});
}

void TextRender::renderLineNumbers(const Font& font,
								   const RenderStyle& renderStyle,
								   const RenderViewPort& viewPort,
								   const BaseFormattedText& text,
								   glm::vec2 position) {
	auto maxLineNumber = numericToString<String>(text.numLines() + 1);
	auto lineNumberSpacing = maxLineNumber.size() * renderStyle.getAdvanceX(font, convertFromChar('0'));
	
	renderView(font, text.numLines(), viewPort, position, [&](GLfloat* vertices,
															  std::size_t& offset,
															  std::function<void()> drawCharacter,
															  std::int64_t lineIndex,
															  const RenderViewPort& currentView,
															  glm::vec2& drawPosition) {
		auto& line = text.getLine((std::size_t)lineIndex);

		if (!line.isContinuation) {
			auto lineNumber = numericToString<String>(line.number + 1);
			auto whitespaceSpacing = renderStyle.getAdvanceX(font, convertFromChar(' '));
			for (std::size_t i = 0; i < maxLineNumber.size() - lineNumber.size(); i++) {
				drawPosition.x += whitespaceSpacing;
			}

			for (auto& character : lineNumber) {
				setCharacterVertices(
					vertices,
					offset,
					font,
					character,
					drawPosition.x,
					drawPosition.y,
					renderStyle.lineNumberColor);

				drawPosition.x += renderStyle.getAdvanceX(font, character);
				drawCharacter();
			}
		}
	});
}

void TextRender::renderCaret(const Font& font,
							 const RenderStyle& renderStyle,
							 const TextMetrics& textMetrics,
							 const RenderViewPort& viewPort,
							 const BaseFormattedText& text,
							 glm::vec2 spacing,
							 const InputState& inputState) {
	setupRendering(font);
	GLfloat charactersVertices[sizeof(GLfloat) * FLOATS_PER_CHARACTER];

	auto& fontCharacter = font['|'];

	auto lineOffset = textMetrics.calculatePositionX(
		text,
		(std::size_t)inputState.caretPositionY,
		(std::size_t)inputState.caretPositionX);

	setCharacterVertices(
		charactersVertices,
		0,
		font,
		'|',
		inputState.viewPosition.x + spacing.x + lineOffset - fontCharacter.size.x,
		inputState.viewPosition.y + spacing.y + (inputState.caretPositionY + 1) * font.lineHeight(),
		renderStyle.textColor);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * FLOATS_PER_CHARACTER, charactersVertices);
	glDrawArrays(GL_TRIANGLES, 0, NUM_TRIANGLES);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}