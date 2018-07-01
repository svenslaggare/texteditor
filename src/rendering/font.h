#pragma once
#include <unordered_map>

#include <glm/vec2.hpp>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

/**
 * Represents a character for a font
 */
struct FontCharacter {
	GLuint textureId;
	glm::ivec2 size;
	glm::ivec2 bearing;
	float advanceX;
};

/**
 * Represents a font
 */
class Font {
private:
	std::uint32_t mSize;
	std::unordered_map<char, FontCharacter> mCharacters;
	GLuint mTextureMap;
	float mLineHeight = 0.0f;

	bool mIsMonoSpace = true;
	float mMonoSpaceAdvanceX = 0.0f;
public:
	/**
	 * Creates a new font
	 * @param name The name of the font
	 * @param size The size of the font
	 */
	Font(const std::string& name, std::uint32_t size);
	~Font();

	/**
	 * Returns the texture map for the font
	 */
	GLuint textureMap() const;

	/**
	 * Returns the height of a line
	 */
	float lineHeight() const;

	/**
	 * Returns the texture coordinates for the given character
	 * @param character The character
	 * @param top The top texture coordinate
	 * @param left The left texture coordinate
	 * @param bottom The bottom texture coordinate
	 * @param right The right texture coordinate
	 */
	void getTextureCoordinates(char character, float& top, float& left, float& bottom, float& right) const;

	/**
	 * Returns the given character
	 * @param character The character
	 */
	const FontCharacter& operator[](char character) const;

	/**
	 * Returns the advance X for the given character
	 * @param character The character
	 */
	float getAdvanceX(char character) const;
};