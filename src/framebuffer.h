#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

/**
 * Represents a frame buffer
 */
class FrameBuffer {
private:
	int mWidth;
	int mHeight;
	GLuint mFrameBuffer;
	GLuint mTextureColorBuffer;
public:
	/**
	 * Creates a new frame buffer
	 * @param width The width of the frame buffer
	 * @param height The height of the frame buffer
	 */
	FrameBuffer(int width, int height);
	~FrameBuffer();

	// Delete copy
	FrameBuffer(const FrameBuffer&) = delete;
	FrameBuffer& operator=(const FrameBuffer&) = default;

	/**
	 * Returns the width of the buffer
	 */
	int width() const;

	/**
	 * Returns the height of the buffer
	 */
	int height() const;

	/**
	 * Returns the texture color buffer
	 */
	GLuint textureColorBuffer() const;

	/**
	 * Binds the current frame buffer
	 */
	void bind();
};