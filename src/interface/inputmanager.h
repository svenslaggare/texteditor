#pragma once
#include "../helpers.h"

#include <glm/vec2.hpp>
#include <unordered_map>
#include <vector>
#include <unordered_set>

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
	std::unordered_set<int> mValidKeys;

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
	 * @param key The key
	 */
	bool isKeyDown(int key);

	/**
	 * Indicates if the given key is being pressed
	 * @param key The key-code for the key
	 */
	bool isKeyPressed(int key);

	/**
	 * Indicates if the shift key is pressed
	 */
	bool isShiftDown();

	/**
	 * Indicates if the shift alt is pressed
	 */
	bool isAltDown();

	/**
	 * Updates the input manager
	 */
	void postUpdate();
};