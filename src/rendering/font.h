#pragma once
#include <unordered_map>
#include <memory>

#include <glm/vec2.hpp>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "../text/text.h"

/**
 * Represents a character for a font
 */
struct FontCharacter {
	glm::ivec2 size;
	glm::ivec2 bearing;
	float advanceX;
	float lineHeight;

	float textureTop;
	float textureLeft;
	float textureBottom;
	float textureRight;
};

/**
 * Represents a font map
 */
class FontMap {
private:
	std::uint32_t mSize;
	std::size_t mCharsPerColumn;
	std::unordered_map<Char, FontCharacter> mCharacters;
	GLuint mTextureMap;
public:
	/**
	 * Creates a new font map
	 * @param name The name of the font
	 * @param size The size of the font
	 * @param characters The characters to load from the font
	 */
	FontMap(const std::string& name, std::uint32_t size, const std::vector<Char>& characters);
	~FontMap();

	/**
	 * Returns the texture map for the font
	 */
	GLuint textureMap() const;

	/**
	 * Returns the characters
	 */
	const std::unordered_map<Char, FontCharacter>& characters() const;
};

/**
 * Represents a font
 */
class Font {
private:
	std::string mName;
	std::uint32_t mSize;

	std::unique_ptr<FontMap> mFontMap;

	float mLineHeight = 0.0f;
	bool mIsMonoSpace = true;
	float mMonoSpaceAdvanceX = 0.0f;

	/**
	 * Creates the font map from the given character
	 * @param characters The characters
	 */
	void createFontMap(const std::vector<Char>& characters);
public:
	/**
	 * Creates a new font
	 * @param name The name of the font
	 * @param size The size of the font
	 */
	Font(const std::string& name, std::uint32_t size);

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
	void getTextureCoordinates(Char character, float& top, float& left, float& bottom, float& right) const;

	/**
	 * Returns the given character
	 * @param character The character
	 */
	const FontCharacter& operator[](Char character) const;

	/**
	 * Returns the advance X for the given character
	 * @param character The character
	 */
	float getAdvanceX(Char character) const;
};