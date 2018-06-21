#include "textrender.h"
#include "font.h"
#include "glhelpers.h"
#include "textformatter.h"
#include "renderstyle.h"
#include "renderviewport.h"
#include "text.h"

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

void TextRender::render(const Font& font,
						FormatMode formatMode,
						const RenderStyle& renderStyle,
						const RenderViewPort& viewPort,
						const Text& text,
						glm::vec2 position) {
	auto& formattedText = text.getFormatted(font, formatMode, renderStyle, viewPort);

	std::size_t linesDrawn = 0;
	std::size_t totalCharactersDrawn = 0;

	glBindVertexArray(mVAO);
	glUseProgram(mShaderProgram);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, font.textureMap());
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);

	GLfloat charactersVertices[sizeof(GLfloat) * FLOATS_PER_CHARACTER * MAX_CHARACTERS];
	std::size_t numCharactersDrawn = 0;

	auto drawCharacters = [&]() {
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * FLOATS_PER_CHARACTER * numCharactersDrawn, charactersVertices);
		glDrawArrays(GL_TRIANGLES, 0, NUM_TRIANGLES * numCharactersDrawn);
		numCharactersDrawn = 0;
	};

	auto cursorLineIndex = (std::int64_t)std::floor(-position.y / font.lineHeight());
	for (std::int64_t lineIndex = cursorLineIndex; lineIndex < (std::int64_t)formattedText.lines.size(); lineIndex++) {
		if (lineIndex >= 0) {
			auto currentX = position.x;
			auto currentY = position.y + (lineIndex + 1) * font.lineHeight();

			if (currentY >= viewPort.top()) {
				for (auto& token : formattedText.lines[lineIndex]) {
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

						numCharactersDrawn++;
						totalCharactersDrawn++;

						if (numCharactersDrawn >= MAX_CHARACTERS) {
							drawCharacters();
						}

						currentX += fontCharacter.advanceX;
					}
				}

				linesDrawn++;
			}

			if (currentY > viewPort.bottom()) {
				break;
			}
		}
	}

//	for (auto& line : formattedText.lines) {
//		if (line.empty()) {
//			currentY += font.lineHeight();
//			continue;
//		}
//
//		auto currentX = position.x;
//
//		float lineHeight = font.lineHeight();
////		for (auto& character : line) {
////			auto& fontCharacter = font[character];
////			lineHeight = std::max(lineHeight, (float)fontCharacter.size.y);
////		}
//
//		currentY += lineHeight;
//		if (currentY >= viewPort.top()) {
//			for (auto& character : line) {
//				auto& fontCharacter = font[character];
//
//				setCharacterVertices(
//					charactersVertices,
//					numCharactersDrawn * FLOATS_PER_CHARACTER,
//					font,
//					character,
//					currentX,
//					currentY);
//
//				numCharactersDrawn++;
//				totalCharactersDrawn++;
//
//				if (numCharactersDrawn >= MAX_CHARACTERS) {
//					drawCharacters();
//				}
//
//				currentX += fontCharacter.advanceX;
//			}
//
//			linesDrawn++;
//		}
//
//		if (currentY > viewPort.bottom()) {
//			break;
//		}
//	}

	if (numCharactersDrawn > 0) {
		drawCharacters();
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

//	std::cout << linesDrawn << std::endl;
//	std::cout << totalCharactersDrawn << std::endl;
}
