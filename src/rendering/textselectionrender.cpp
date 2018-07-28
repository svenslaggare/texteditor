#include "textselectionrender.h"
#include "shadercompiler.h"
#include "font.h"
#include "renderstyle.h"

#include "../helpers.h"
#include "../windowstate.h"

#include "../interface/textview.h"

#include <glm/gtc/matrix_transform.hpp>

TextSelectionRender::TextSelectionRender()
	: mShaderProgram(Helpers::readFileAsUTF8Text("shaders/selectionVertex.glsl"), Helpers::readFileAsUTF8Text("shaders/selection.glsl")) {
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 5 * 6, nullptr, GL_DYNAMIC_DRAW);

	glUseProgram(mShaderProgram.id());

	auto posAttribute = glGetAttribLocation(mShaderProgram.id(), "vertexPosition");
	glEnableVertexAttribArray(posAttribute);
	glVertexAttribPointer(posAttribute, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);

	auto colorAttribute = glGetAttribLocation(mShaderProgram.id(), "vertexColor");
	glEnableVertexAttribArray(colorAttribute);
	glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

TextSelectionRender::~TextSelectionRender() {
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

void TextSelectionRender::render(const WindowState& windowState,
								 const Font& font,
								 const TextMetrics& textMetrics,
								 const BaseFormattedText& formattedText,
								 glm::vec2 offset,
								 const InputState& inputState) {
//	if (inputState.selection.isSingle()) {
//		return;
//	}

	glUseProgram(mShaderProgram.id());
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	mShaderProgram.setParameters({ ShaderParameter::float4x4MatrixParameter("projection", windowState.projection()) });

	for (auto selectionLineIndex = inputState.selection.startY; selectionLineIndex <= inputState.selection.endY; selectionLineIndex++) {
		auto& line = formattedText.getLine(selectionLineIndex);

		std::size_t selectionLineCharStartIndex = 0;
		std::size_t selectionLineCharEndIndex = line.length();
		bool isLastLine = false;

		if (selectionLineIndex == inputState.selection.startY) {
			selectionLineCharStartIndex = inputState.selection.startX;
		}

		if (selectionLineIndex == (inputState.selection.endY)) {
			selectionLineCharEndIndex = inputState.selection.endX;
			isLastLine = true;
		}

		auto lineOffset = textMetrics.calculatePositionX(
			formattedText,
			selectionLineIndex,
			selectionLineCharStartIndex);

		auto& fontCharacter = font['|'];
		auto selectionPositionX = offset.x + lineOffset;
		auto selectionPositionY = -(offset.y + selectionLineIndex * font.lineHeight()) - (fontCharacter.size.y - fontCharacter.bearing.y);

		float selectionLineWidth = textMetrics.getLineWidth(
			line,
			selectionLineCharStartIndex,
			&selectionLineCharEndIndex);

		if (!isLastLine) {
			selectionLineWidth = windowState.width();
		}

//		if (selectionLineCharStartIndex == selectionLineCharEndIndex) {
//			selectionLineWidth = 0.0f;
//		}

		float selectionLineHeight = font.lineHeight();

		GLfloat vertices[] = {
			selectionPositionX, selectionPositionY, 0.0f, 0.0f, 0.0f,  // Top-left
			selectionPositionX + selectionLineWidth, selectionPositionY, 0.0f, 0.0f, 0.0f, // Top-right
			selectionPositionX, selectionPositionY - selectionLineHeight, 0.0f, 0.0f, 0.0f, // Bottom-left

			selectionPositionX, selectionPositionY - selectionLineHeight, 0.0f, 0.0f, 0.0f, // Bottom-left
			selectionPositionX + selectionLineWidth, selectionPositionY - selectionLineHeight, 0.0f, 0.0f, 0.0f, // Bottom-right
			selectionPositionX + selectionLineWidth, selectionPositionY, 0.0f, 0.0f, 0.0f, // Top-right
		};

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}
