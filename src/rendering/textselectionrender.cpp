#include "textselectionrender.h"
#include "shadercompiler.h"
#include "font.h"
#include "renderstyle.h"

#include "../helpers.h"
#include "../windowstate.h"

#include "../interface/textview.h"

#include <glm/gtc/matrix_transform.hpp>

TextSelectionRender::TextSelectionRender() {
	mVertexShader = ShaderCompiler::loadAndCompileShader(Helpers::readFileAsUTF8Text("shaders/selectionVertex.glsl"), GL_VERTEX_SHADER);
	mFragmentShader = ShaderCompiler::loadAndCompileShader(Helpers::readFileAsUTF8Text("shaders/selection.glsl"), GL_FRAGMENT_SHADER);
	mSelectionShaderProgram = ShaderCompiler::linkShaders(mVertexShader, mFragmentShader);

	glGenVertexArrays(1, &mSelectionVAO);
	glGenBuffers(1, &mSelectionVBO);
	glBindVertexArray(mSelectionVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mSelectionVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 5 * 6, nullptr, GL_DYNAMIC_DRAW);

	glUseProgram(mSelectionShaderProgram);

	auto posAttribute = glGetAttribLocation(mSelectionShaderProgram, "vertexPosition");
	glEnableVertexAttribArray(posAttribute);
	glVertexAttribPointer(posAttribute, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);

	auto colorAttribute = glGetAttribLocation(mSelectionShaderProgram, "vertexColor");
	glEnableVertexAttribArray(colorAttribute);
	glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

TextSelectionRender::~TextSelectionRender() {
	glDeleteShader(mVertexShader);
	glDeleteShader(mFragmentShader);
	glDeleteProgram(mSelectionShaderProgram);

	glDeleteVertexArrays(1, &mSelectionVAO);
	glDeleteBuffers(1, &mSelectionVBO);
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

	glUseProgram(mSelectionShaderProgram);

	auto projection = glm::ortho(0.0f, (float)windowState.width(), -(float)windowState.height(), 0.0f);
	glUniformMatrix4fv(glGetUniformLocation(mSelectionShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

	glBindVertexArray(mSelectionVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mSelectionVBO);

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
