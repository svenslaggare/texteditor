#pragma once
#include "helpers.h"

#include <glm/vec2.hpp>
#include <unordered_map>

class GLFWwindow;
class WindowState;
class RenderViewPort;
class Font;

/**
 * Represents an input manager
 */
class InputManager {
private:
	GLFWwindow* mWindow;

	std::unordered_map<int, TimePoint> mLastDown;
	std::unordered_map<int, TimePoint> mLastPressed;
	std::unordered_map<int, int> mPreviousState;
public:
	/**
	 * Creates a new input manager
	 * @param window The window
	 */
	explicit InputManager(GLFWwindow* window);

	/**
	 * Indicates if the given key is down
	 * @param key The key-code for the key
	 */
	bool isKeyPressed(int key);

	/**
	 * Updates the input manager
	 */
	void postUpdate();
};