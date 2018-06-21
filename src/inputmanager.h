#pragma once

#include <glm/vec2.hpp>

class GLFWwindow;
class WindowState;

/**
 * Represents an input manager
 */
class InputManager {
private:
	GLFWwindow* mWindow;

	glm::vec2 mCursorPosition;

	const int mScrollSpeed = 80;
	const int mPageMoveSpeed = 200;
	const int mKeyMoveSpeed = 20;
public:
	/**
	 * Creates a new input manager
	 * @param window The window
	 */
	explicit InputManager(GLFWwindow* window);

	/**
	 * Returns the position of the cursor
	 */
	glm::vec2 cursorPosition() const;

	/**
	 * Updates the input manager
	 */
	void update(WindowState& windowState);
};