#pragma once
#include <glm/vec3.hpp>

struct Token;

/**
 * The render style
 */
struct RenderStyle {
	float sideSpacing = 10.0f;
	float topSpacing = 5.0f;
	float bottomSpacing = 0.0f;
	int spacesPerTab = 4;

	bool wordWrap = true;

	glm::vec3 backgroundColor = glm::vec3(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f);
	glm::vec3 textColor = glm::vec3(212.0f / 255.0f, 212.0f / 255.0f, 212.0f / 255.0f);
	glm::vec3 keywordColor = glm::vec3(65.0f / 255.0f, 148.0f / 255.0f, 214.0f / 255.0f);
	glm::vec3 stringColor = glm::vec3(206.0f / 255.0f, 145.0f / 255.0f, 97.0f / 255.0f);
	glm::vec3 commentColor = glm::vec3(73.0f / 255.0f, 132.0f / 255.0f, 78.0f / 255.0f);

	/**
	 * Returns the color of the given token
	 * @param token The token
	 */
	glm::vec3 getColor(const Token& token) const;
};