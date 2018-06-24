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
	: mWindow(window), mViewPosition(0.0f, 0.0f) {

}

InputState InputManager::getInputState() const {
	return { mViewPosition, mCaretPositionX, mCaretPositionY };
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
		if (Helpers::durationMS(Helpers::timeNow(), lastDownIterator->second) >= 200) {
			auto lastPressedIterator = mLastPressed.find(key);
			if (lastPressedIterator != mLastPressed.end()) {
				if (Helpers::durationMS(Helpers::timeNow(), lastPressedIterator->second) >= 30) {
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

void InputManager::update(WindowState& windowState, const Font& font, const RenderViewPort& viewPort) {
	auto lineHeight = font.lineHeight();

	if (glfwGetKey(mWindow, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
		mViewPosition.y += mPageMoveSpeed;
	}

	if (glfwGetKey(mWindow, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
		mViewPosition.y -= mPageMoveSpeed;
	}

	int caretPositionDiffY = 0;

	if (isKeyPressed(GLFW_KEY_UP)) {
		caretPositionDiffY = -1;
	}

	if (isKeyPressed(GLFW_KEY_DOWN)) {
		caretPositionDiffY = 1;
	}

	if (caretPositionDiffY != 0) {
		mCaretPositionY += caretPositionDiffY;
		mCaretPositionY = std::max(mCaretPositionY, 0L);

		auto caretScreenPositionY = -std::max(mCaretPositionY + caretPositionDiffY, 0L) * lineHeight;
		if (caretScreenPositionY < mViewPosition.y - viewPort.height) {
			mViewPosition.y -= caretPositionDiffY * lineHeight;
		}

		if (caretScreenPositionY > mViewPosition.y) {
			mViewPosition.y -= caretPositionDiffY * lineHeight;
		}

		if (!(-mViewPosition.y + viewPort.height >= -caretScreenPositionY && -mViewPosition.y <= -caretScreenPositionY)) {
			mViewPosition.y = -mCaretPositionY * lineHeight + viewPort.height / 2.0f;
		}
	}

	int caretPositionDiffX = 0;
	if (isKeyPressed(GLFW_KEY_LEFT)) {
		caretPositionDiffX = -1;
	}

	if (isKeyPressed(GLFW_KEY_RIGHT)) {
		caretPositionDiffX = 1;
	}

	if (caretPositionDiffX != 0) {
		mCaretPositionX += caretPositionDiffX;
		mCaretPositionX = std::max(mCaretPositionX, 0L);

		auto lineWidth = font['A'].advanceX;

		auto caretScreenPositionX = -std::max(mCaretPositionX + caretPositionDiffX, 0L) * lineWidth;
		if (caretScreenPositionX < mViewPosition.x - (viewPort.width - 60)) {
			mViewPosition.x -= caretPositionDiffX * lineWidth;
		}

		if (caretScreenPositionX > mViewPosition.x) {
			mViewPosition.x -= caretPositionDiffX * lineWidth;
		}
	}

	if (windowState.scrolled) {
		mViewPosition.y += mScrollSpeed * lineHeight * windowState.scrollY;
		windowState.scrolled = false;

		if (mViewPosition.y > 0) {
			mViewPosition.y = 0;
		}
	}

	for (auto& key : keys) {
		mPreviousState[key] = glfwGetKey(mWindow, key);
	}
}
