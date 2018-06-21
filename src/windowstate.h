#pragma once

/**
 * Represents the state of the window
 */
struct WindowState {
	int width = 1280;
	int height = 720;
	bool changed = false;

	double scrollY = 0.0;
	bool scrolled = false;
};