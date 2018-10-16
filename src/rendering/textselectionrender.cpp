#include "textselectionrender.h"
#include "common/shadercompiler.h"
#include "font.h"
#include "renderstyle.h"

#include "../helpers.h"
#include "../windowstate.h"

#include "../interface/textview.h"

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
								 const RenderViewPort& viewPort,
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

//	auto startTime = Helpers::timeNow();
	auto cursorLineIndex = (std::size_t)std::floor(std::max(-offset.y / font.lineHeight(), 0.0f));
	for (auto selectionLineIndex = std::max(cursorLineIndex, inputState.selection.startY); selectionLineIndex <= inputState.selection.endY; selectionLineIndex++) {
		std::size_t selectionLineCharStartIndex = 0;
		auto lineOffset = 0.0f;

		float selectionLineWidth = windowState.width();
		float selectionLineHeight = font.lineHeight();

		// Note: formatted line only guaranteed to be available for first and last line.
		if (selectionLineIndex == inputState.selection.startY) {
			selectionLineCharStartIndex = inputState.selection.startX;
			lineOffset = textMetrics.calculatePositionX(
				formattedText,
				selectionLineIndex,
				selectionLineCharStartIndex);
		}

		if (selectionLineIndex == (inputState.selection.endY)) {
			auto& line = formattedText.getLine(selectionLineIndex);
			auto selectionLineCharEndIndex = inputState.selection.endX;
			selectionLineWidth = textMetrics.getLineWidth(
				line,
				selectionLineCharStartIndex,
				&selectionLineCharEndIndex);
		}

		// Calculate position of bounding box line
		auto& fontCharacter = font['|'];
		auto selectionPositionX = offset.x + lineOffset;
		auto drawPositionY = offset.y + selectionLineIndex * font.lineHeight();
		auto selectionPositionY = -drawPositionY - (fontCharacter.size.y - fontCharacter.bearing.y);

		if (drawPositionY > viewPort.bottom()) {
			break;
		}

		//TODO: batch draw calls maybe
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

//	std::cout << "Render time: " << Helpers::durationMilliseconds(Helpers::timeNow(), startTime) << std::endl;
}
