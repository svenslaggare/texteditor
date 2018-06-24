#pragma once
#include "helpers.h"

#include <glm/vec2.hpp>
#include <unordered_map>


class GLFWwindow;
class WindowState;
class RenderViewPort;
class Font;

/**
 * Returns the input state
 */
struct InputState {
	glm::vec2 viewPosition = glm::vec2();
	std::int64_t caretPositionX = 0;
	std::int64_t caretPositionY = 0;
};

/**
 * Represents an input manager
 */
class InputManager {
private:
	GLFWwindow* mWindow;

	glm::vec2 mViewPosition;
	std::int64_t mCaretPositionX = 0;
	std::int64_t mCaretPositionY = 0;

	std::unordered_map<int, TimePoint> mLastDown;
	std::unordered_map<int, TimePoint> mLastPressed;
	std::unordered_map<int, int> mPreviousState;

	const float mScrollSpeed = 4.0f;
	const int mPageMoveSpeed = 200;

	/**
	 * Indicates if the given key is down
	 * @param key The key-code for the key
	 */
	bool isKeyPressed(int key);
public:
	/**
	 * Creates a new input manager
	 * @param window The window
	 */
	explicit InputManager(GLFWwindow* window);

	/**
	 * Returns the input state
	 */
	InputState getInputState() const;

	/**
	 * Updates the input manager
	 * @param windowState The window state
	 * @param font The font
	 * @param viewPort The view port
	 */
	void update(WindowState& windowState, const Font& font, const RenderViewPort& viewPort);
};