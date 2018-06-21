#include "inputmanager.h"
#include "windowstate.h"

#include <GLFW/glfw3.h>
#include <algorithm>

InputManager::InputManager(GLFWwindow* window)
	: mWindow(window), mCursorPosition(0.0f, 0.0f) {

}

glm::vec2 InputManager::cursorPosition() const {
	return mCursorPosition;
}

void InputManager::update(WindowState& windowState) {
	if (glfwGetKey(mWindow, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
		mCursorPosition.y += mPageMoveSpeed;
	}

	if (glfwGetKey(mWindow, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
		mCursorPosition.y -= mPageMoveSpeed;
	}

	if (glfwGetKey(mWindow, GLFW_KEY_UP) == GLFW_PRESS) {
		mCursorPosition.y  += mKeyMoveSpeed;
	}

	if (glfwGetKey(mWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
		mCursorPosition.y -= mKeyMoveSpeed;
	}

	if (glfwGetKey(mWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
		mCursorPosition.x += mKeyMoveSpeed;
		mCursorPosition.x = std::min(mCursorPosition.x, 0.0f);
	}

	if (glfwGetKey(mWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		mCursorPosition.x -= mKeyMoveSpeed;
	}

	if (windowState.scrolled) {
		mCursorPosition.y += mScrollSpeed * windowState.scrollY;
		windowState.scrolled = false;
	}
}
