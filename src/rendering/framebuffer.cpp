#include "framebuffer.h"

FrameBuffer::FrameBuffer(int width, int height)
	: mWidth(width),
	  mHeight(height) {
	glGenFramebuffers(1, &mFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);

	glGenTextures(1, &mTextureColorBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTextureColorBuffer);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		width,
		height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		nullptr
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		mTextureColorBuffer,
		0
	);
}

FrameBuffer::~FrameBuffer() {
	glDeleteFramebuffers(1, &mFrameBuffer);
	glDeleteTextures(1, &mTextureColorBuffer);
}

int FrameBuffer::width() const {
	return mWidth;
}

int FrameBuffer::height() const {
	return mHeight;
}

GLuint FrameBuffer::textureColorBuffer() const {
	return mTextureColorBuffer;
}

void FrameBuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
}
