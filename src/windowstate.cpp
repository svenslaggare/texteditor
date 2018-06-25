#include "windowstate.h"

void WindowState::update() {
	if (mScrollValueChanged) {
		mScrollValueChanged = false;
		mHasScrolled = true;
	} else {
		mHasScrolled = false;
		mScrollValueY = 0.0;
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
