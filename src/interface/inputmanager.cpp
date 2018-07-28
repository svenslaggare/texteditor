#include "inputmanager.h"
#include "../windowstate.h"
#include "../rendering/renderviewport.h"
#include "../rendering/font.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

InputManager::InputManager(GLFWwindow* window)
	: mWindow(window) {

}

bool InputManager::isKeyDown(int key) {
	return glfwGetKey(mWindow, key) == GLFW_PRESS;
}

bool InputManager::isKeyPressed(int key) {
	if (mValidKeys.count(key) == 0) {
		mValidKeys.insert(key);
	}

	auto prevState = mPreviousState.find(key);
	bool currentPressed = glfwGetKey(mWindow, key) == GLFW_PRESS;
	bool prevReleased = prevState != mPreviousState.end() && prevState->second == GLFW_RELEASE;
	bool prevPressed = prevState != mPreviousState.end() && prevState->second == GLFW_PRESS;

	if (currentPressed && !prevPressed) {
		mLastDown[key] = Helpers::timeNow();
	}

	if (currentPressed && prevReleased) {
		return true;
	}

	auto lastDownIterator = mLastDown.find(key);
	if (currentPressed && lastDownIterator != mLastDown.end()) {
		if (Helpers::durationMilliseconds(Helpers::timeNow(), lastDownIterator->second) >= 200) {
			auto lastPressedIterator = mLastPressed.find(key);
			if (lastPressedIterator != mLastPressed.end()) {
				if (Helpers::durationMilliseconds(Helpers::timeNow(), lastPressedIterator->second) >= 30) {
					mLastPressed[key] = Helpers::timeNow();
					return true;
				}
			} else {
				mLastPressed[key] = Helpers::timeNow();
				return true;
			}
		}
	}

	return false;
}

bool InputManager::isShiftDown() {
	return isKeyDown(GLFW_KEY_LEFT_SHIFT) || isKeyDown(GLFW_KEY_RIGHT_SHIFT);
}

bool InputManager::isAltDown() {
	return isKeyDown(GLFW_KEY_LEFT_ALT) || isKeyDown(GLFW_KEY_RIGHT_ALT);
}

bool InputManager::isMouseButtonHoldDown(int button, int timeMs) {
	auto prevState = mMousePreviousState.find(button);
	bool currentPressed = glfwGetMouseButton(mWindow, button) == GLFW_PRESS;
	bool prevReleased = prevState != mMousePreviousState.end() && prevState->second == GLFW_RELEASE;

	if (currentPressed && prevReleased) {
		mMouseLastDown[button] = Helpers::timeNow();
	}

	auto lastDownIterator = mMouseLastDown.find(button);
	if (currentPressed && lastDownIterator != mMouseLastDown.end()) {
		return Helpers::durationMilliseconds(Helpers::timeNow(), lastDownIterator->second) >= timeMs;
	}

	return false;
}

bool InputManager::isMouseDragMove(int button) {
	double mouseX;
	double mouseY;
	glfwGetCursorPos(mWindow, &mouseX, &mouseY);

	bool isDragMove = (std::abs(mouseX - mLastMoveMouseX) > 1E-4 || std::abs(mouseY - mLastMoveMouseY) > 1E-4)
		   				&& glfwGetMouseButton(mWindow, button) == GLFW_PRESS;

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (isDragMove) {
			mIsLeftMouseButtonDragMove = isDragMove;
		} else {
			isDragMove = mIsLeftMouseButtonDragMove;
		}
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (isDragMove) {
			mIsRightMouseButtonDragMove = isDragMove;
		} else {
			isDragMove = mIsRightMouseButtonDragMove;
		}
	}

	return isDragMove;
}

void InputManager::postUpdate() {
	for (auto& key : mValidKeys) {
		mPreviousState[key] = glfwGetKey(mWindow, key);
	}

	mMousePreviousState[GLFW_MOUSE_BUTTON_LEFT] = glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_LEFT);
	mMousePreviousState[GLFW_MOUSE_BUTTON_RIGHT] = glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_RIGHT);

	if (mIsLeftMouseButtonDragMove && mMousePreviousState[GLFW_MOUSE_BUTTON_LEFT] == GLFW_RELEASE) {
		mIsLeftMouseButtonDragMove = false;
	}

	if (mIsRightMouseButtonDragMove && mMousePreviousState[GLFW_MOUSE_BUTTON_RIGHT] == GLFW_RELEASE) {
		mIsRightMouseButtonDragMove = false;
	}

	glfwGetCursorPos(mWindow, &mLastMoveMouseX, &mLastMoveMouseY);
}
