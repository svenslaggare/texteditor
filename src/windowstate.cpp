#include "windowstate.h"
#include <glm/gtc/matrix_transform.hpp>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace {
	WindowState& getWindowState(GLFWwindow* window) {
		return *(WindowState*)glfwGetWindowUserPointer(window);
	}
}

void WindowState::initialize(GLFWwindow* window) {
	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		auto& windowState = getWindowState(window);
		windowState.changeWindowSize(width, height);
		glViewport(0, 0, width, height);
	});

	glfwSetScrollCallback(window, [](GLFWwindow* window, double offsetX, double offsetY) {
		auto& windowState = getWindowState(window);
		windowState.setScrollY(offsetY);
	});

	glfwSetCharCallback(window, [](GLFWwindow* window, CodePoint codePoint) {
		auto& windowState = getWindowState(window);
		windowState.addCharacter(codePoint);
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
		auto& windowState = getWindowState(window);

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			windowState.leftMouseButtonPressed();
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			windowState.rightMouseButtonPressed();
		}
	});
}

void WindowState::update() {
	if (mScrollValueChanged) {
		mScrollValueChanged = false;
		mHasScrolled = true;
	} else {
		mHasScrolled = false;
		mScrollValueY = 0.0;
	}

	mInputCharacters = std::move(mCharacterBuffer);
	mCharacterBuffer = {};

	if (mLeftMouseButtonPressedChanged) {
		mLeftMouseButtonPressed = true;
		mLeftMouseButtonPressedChanged = false;
	} else {
		mLeftMouseButtonPressed = false;
	}

	if (mRightMouseButtonPressedChanged) {
		mRightMouseButtonPressed = true;
		mRightMouseButtonPressedChanged = false;
	} else {
		mRightMouseButtonPressed = false;
	}
}

void WindowState::changeWindowSize(int width, int height) {
	mWidth = width;
	mHeight = height;
	mChangedWindowSize = true;
}

int WindowState::width() const {
	return mWidth;
}

int WindowState::height() const {
	return mHeight;
}

glm::mat4x4 WindowState::projection() const {
	return glm::ortho(0.0f, (float)width(), -(float)height(), 0.0f);
}

bool WindowState::hasChangedWindowSize() {
	if (mChangedWindowSize) {
		mChangedWindowSize = false;
		return true;
	}

	return false;
}

void WindowState::setScrollY(double value) {
	mScrollValueY = value;
	mScrollValueChanged = true;
}

bool WindowState::hasScrolled() const {
	return mHasScrolled;
}

double WindowState::scrollY() const {
	return mScrollValueY;
}

void WindowState::addCharacter(CodePoint codePoint) {
	mCharacterBuffer.push_back(codePoint);
}

const std::vector<CodePoint>& WindowState::inputCharacters() const {
	return mInputCharacters;
}

void WindowState::leftMouseButtonPressed() {
	mLeftMouseButtonPressedChanged = true;
}

bool WindowState::isLeftMouseButtonPressed() const {
	return mLeftMouseButtonPressed;
}

void WindowState::rightMouseButtonPressed() {
	mRightMouseButtonPressedChanged = true;
}

bool WindowState::isRightMouseButtonPressed() const {
	return mRightMouseButtonPressed;
}
