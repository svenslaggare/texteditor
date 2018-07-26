#include "windowstate.h"

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
