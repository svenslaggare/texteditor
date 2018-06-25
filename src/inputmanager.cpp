#include "inputmanager.h"
#include "windowstate.h"
#include "rendering/renderviewport.h"
#include "rendering/font.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

namespace {
	const std::vector<int> keys = {
		GLFW_KEY_LEFT,
		GLFW_KEY_RIGHT,
		GLFW_KEY_UP,
		GLFW_KEY_DOWN,
		GLFW_KEY_PAGE_UP,
		GLFW_KEY_PAGE_DOWN
	};
}

InputManager::InputManager(GLFWwindow* window)
	: mWindow(window) {

}

bool InputManager::isKeyPressed(int key) {
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

void InputManager::postUpdate() {
	for (auto& key : keys) {
		mPreviousState[key] = glfwGetKey(mWindow, key);
	}
}
