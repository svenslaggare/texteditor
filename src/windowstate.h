#pragma once

/**
 * Represents the state of the window
 */
class WindowState {
private:
	int mWidth = 1280;
	int mHeight = 720;
	bool mChangedWindowSize = false;

	double mScrollValueY = 0.0;
	bool mScrollValueChanged = false;

	bool mHasScrolled = false;
public:
	/**
	 * Updates the state. Should be called in the start of the frame.
	 */
	void update();

	/**
	 * Changes the window size
	 * @param width The width of the window
	 * @param height The height of the window
	 */
	void changeWindowSize(int width, int height);

	/**
	 * Returns the width of the window
	 */
	int width() const;

	/**
	 * Returns the height of the window
	 */
	int height() const;

	/**
	 * Indicates if the window size has changed, if that the case, clears the indication
	 */
	bool hasChangedWindowSize();

	/**
	 * Sets the scroll value
	 */
	void setScrollY(double value);

	/**
	 * Indicates if we have scrolled in the current frame
	 */
	bool hasScrolled() const;

	/**
	 * Returns the Y scroll value
	 */
	double scrollY() const;
};