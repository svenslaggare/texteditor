#include "textrender.h"
#include "font.h"
#include "glhelpers.h"
#include "../text/textformatter.h"
#include "renderstyle.h"
#include "renderviewport.h"
#include "../text/text.h"

#include <vector>
#include <iostream>
#include <glm/vec3.hpp>
#include <cmath>

namespace {
	const std::size_t NUM_TRIANGLES = 6;
	const std::size_t MAX_CHARACTERS = 200;
	const std::size_t FLOATS_PER_CHARACTER = NUM_TRIANGLES * (2 + 2 + 3);
}

TextRender::TextRender(GLuint shaderProgram)
	: mShaderProgram(shaderProgram) {
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * FLOATS_PER_CHARACTER * MAX_CHARACTERS, nullptr, GL_DYNAMIC_DRAW);

	auto vertexSize = (7) * sizeof(GLfloat);
	auto posAttrib = glGetAttribLocation(shaderProgram, "vertexPosition");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, vertexSize, nullptr);

	auto texAttrib = glGetAttribLocation(shaderProgram, "vertexTexcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)(2 * sizeof(GLfloat)));

	auto colorAttrib = glGetAttribLocation(shaderProgram, "vertexColor");
	glEnableVertexAttribArray(colorAttrib);
	glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)((2 + 2) * sizeof(GLfloat)));

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
									  char character,
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

void TextRender::initializeRendering(const Font& font) {
	glBindVertexArray(mVAO);
	glUseProgram(mShaderProgram);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, font.textureMap());
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
}

void TextRender::renderCurrentView(const Font& font,
								   std::size_t maxNumLines,
								   const RenderViewPort& viewPort,
								   glm::vec2 position,
								   std::function<void (std::int64_t, float&, float&)> renderLine) {
	auto cursorLineIndex = (std::int64_t)std::floor(-position.y / font.lineHeight());
	for (std::int64_t lineIndex = cursorLineIndex; lineIndex < (std::int64_t)maxNumLines; lineIndex++) {
		if (lineIndex >= 0) {
			auto currentX = position.x;
			auto currentY = position.y + (lineIndex + 1) * font.lineHeight();

			if (currentY >= viewPort.top()) {
				renderLine(lineIndex, currentX, currentY);
			}

			if (currentY > viewPort.bottom()) {
				break;
			}
		}
	}
}

void TextRender::render(const Font& font,
						const RenderStyle& renderStyle,
						const RenderViewPort& viewPort,
						const FormattedText& text,
						glm::vec2 position) {
	initializeRendering(font);

	GLfloat charactersVertices[sizeof(GLfloat) * FLOATS_PER_CHARACTER * MAX_CHARACTERS];
	std::size_t numCharactersDrawn = 0;

	auto drawCharacters = [&]() {
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * FLOATS_PER_CHARACTER * numCharactersDrawn, charactersVertices);
		glDrawArrays(GL_TRIANGLES, 0, NUM_TRIANGLES * numCharactersDrawn);
		numCharactersDrawn = 0;
	};

	renderCurrentView(font, text.lines.size(), viewPort, position, [&](std::int64_t lineIndex, float& currentX, float& currentY) {
		for (auto& token : text.lines[lineIndex]) {
			auto color = renderStyle.getColor(token);

			for (auto& character : token.text) {
				auto& fontCharacter = font[character];

				setCharacterVertices(
					charactersVertices,
					numCharactersDrawn * FLOATS_PER_CHARACTER,
					font,
					character,
					currentX,
					currentY,
					color);

				currentX += fontCharacter.advanceX;

				numCharactersDrawn++;
				if (numCharactersDrawn >= MAX_CHARACTERS) {
					drawCharacters();
				}
			}
		}
	});

	if (numCharactersDrawn > 0) {
		drawCharacters();
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TextRender::renderLineNumbers(const Font& font,
								   const RenderStyle& renderStyle,
								   const RenderViewPort& viewPort,
								   std::size_t maxNumberLines,
								   glm::vec2 position) {
	initializeRendering(font);

	GLfloat charactersVertices[sizeof(GLfloat) * FLOATS_PER_CHARACTER * MAX_CHARACTERS];
	std::size_t numCharactersDrawn = 0;

	auto drawCharacters = [&]() {
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * FLOATS_PER_CHARACTER * numCharactersDrawn, charactersVertices);
		glDrawArrays(GL_TRIANGLES, 0, NUM_TRIANGLES * numCharactersDrawn);
		numCharactersDrawn = 0;
	};

	renderCurrentView(font, maxNumberLines, viewPort, position, [&](std::int64_t lineIndex, float& currentX, float& currentY) {
		auto lineNumber = std::to_string(lineIndex + 1);
		for (auto& character : lineNumber) {
			auto& fontCharacter = font[character];

			setCharacterVertices(
				charactersVertices,
				numCharactersDrawn * FLOATS_PER_CHARACTER,
				font,
				character,
				currentX,
				currentY,
				renderStyle.lineNumberColor);

			currentX += fontCharacter.advanceX;

			numCharactersDrawn++;
			if (numCharactersDrawn >= MAX_CHARACTERS) {
				drawCharacters();
			}
		}
	});

	if (numCharactersDrawn > 0) {
		drawCharacters();
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
