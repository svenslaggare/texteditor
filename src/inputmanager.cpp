#include "inputmanager.h"
#include "windowstate.h"
#include "rendering/renderviewport.h"
#include "rendering/font.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

InputManager::InputManager(GLFWwindow* window)
	: mWindow(window) {
	mValidKeys.push_back(GLFW_KEY_LEFT);
	mValidKeys.push_back(GLFW_KEY_RIGHT);
	mValidKeys.push_back(GLFW_KEY_UP);
	mValidKeys.push_back(GLFW_KEY_DOWN);
	mValidKeys.push_back(GLFW_KEY_PAGE_UP);
	mValidKeys.push_back(GLFW_KEY_PAGE_DOWN);

	for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
		mValidKeys.push_back(key);
	}

	for (int key = GLFW_KEY_0; key <= GLFW_KEY_9; key++) {
		mValidKeys.push_back(key);
	}

	mValidKeys.push_back(GLFW_KEY_SPACE);
	mValidKeys.push_back(GLFW_KEY_BACKSPACE);
	mValidKeys.push_back(GLFW_KEY_DELETE);
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
	for (auto& key : mValidKeys) {
		mPreviousState[key] = glfwGetKey(mWindow, key);
	}
}
