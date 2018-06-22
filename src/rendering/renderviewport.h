#pragma once
#include <glm/vec2.hpp>

struct RenderViewPort {
	glm::vec2 position;
	float width;
	float height;

	/**
	 * Returns the top Y position
	 */
	float top() const;

	/**
	 * Returns the bottom Y position
	 */
	float bottom() const;
};