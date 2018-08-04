#pragma once
#include "../helpers.h"

#include <glm/vec2.hpp>
#include <unordered_map>
#include <vector>
#include <unordered_set>

struct GLFWwindow;
class WindowState;
struct RenderViewPort;
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

	std::unordered_map<int, TimePoint> mMouseLastDown;
	std::unordered_map<int, int> mMousePreviousState;

	double mLastMoveMouseX = 0.0;
	double mLastMoveMouseY = 0.0;

	bool mIsLeftMouseButtonDragMove = false;
	bool mIsRightMouseButtonDragMove = false;
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
	 * Indicates if the control key is pressed
	 */
	bool isControlDown();

	/**
	 * Indicates if the shift key is pressed
	 */
	bool isShiftDown();

	/**
	 * Indicates if the shift alt is pressed
	 */
	bool isAltDown();

	/**
	 * Indicates if the given mouse button has been held down for the given amount of time
	 * @param button The button
	 * @param timeMs The time in milliseconds
	 */
	bool isMouseButtonHoldDown(int button, int timeMs);

	/**
	 * Indicates if the given mouse button is drag moved
	 * @param button The button
	 */
	bool isMouseDragMove(int button);

	/**
	 * Updates the input manager
	 */
	void postUpdate();
};