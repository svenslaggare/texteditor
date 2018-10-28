#pragma once
#include <glm/vec3.hpp>
#include "../text/text.h"
#include "font.h"

struct Token;
class Font;

/**
 * The render style
 */
struct RenderStyle {
	float sideSpacing = 10.0f;
	float topSpacing = 5.0f;
	float bottomSpacing = 0.0f;
	std::size_t spacesPerTab = 4;

	bool wordWrap = false;

	glm::vec3 backgroundColor = glm::vec3(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f);
	glm::vec3 textColor = glm::vec3(212.0f / 255.0f, 212.0f / 255.0f, 212.0f / 255.0f);
	glm::vec3 keywordColor = glm::vec3(65.0f / 255.0f, 148.0f / 255.0f, 214.0f / 255.0f);
	glm::vec3 stringColor = glm::vec3(206.0f / 255.0f, 145.0f / 255.0f, 97.0f / 255.0f);
	glm::vec3 numberColor = glm::vec3(181.0f / 255.0f, 206.0f / 255.0f, 168.0f / 255.0f);
	glm::vec3 commentColor = glm::vec3(73.0f / 255.0f, 132.0f / 255.0f, 78.0f / 255.0f);
	glm::vec3 lineNumberColor = glm::vec3(76.0f / 255.0f, 76.0f / 255.0f, 76.0f / 255.0f);

	/**
	 * Returns the color of the given token
	 * @param token The token
	 */
	glm::vec3 getColor(const Token& token) const;

	/**
	 * Returns the advance for the given character in the given font
	 * @param font The font
	 * @param character The character
	 */
	inline float getAdvanceX(const Font& font, Char character) const {
		auto advanceX = font.getAdvanceX(character);

		if (character == '\t') {
			return advanceX * spacesPerTab;
		}

		return advanceX;
	}
};