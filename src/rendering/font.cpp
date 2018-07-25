#include "font.h"
#include <iostream>
#include <memory>

namespace {
	const auto numChars = 256;
}

Font::Font(const std::string& name, std::uint32_t size)
	: mSize(size) {
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		throw std::runtime_error("Could not init FreeType Library.");
	}

	FT_Face face;
	if (FT_New_Face(ft, name.c_str(), 0, &face)) {
		throw std::runtime_error("Failed to load font.");
	}

	FT_Set_Pixel_Sizes(face, 0, size);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

	// Determine size of the elements in the map
	for (unsigned int character = 0; character < numChars; character++) {
		// Load font character glyph
		if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
			continue;
		}

		mSize = std::max(mSize, face->glyph->bitmap.width);
		mSize = std::max(mSize, face->glyph->bitmap.rows);
	}

	mCharsPerColumn = numChars;
	const auto maxWidth = numChars * (float)mSize;

	auto buffer = std::make_unique<std::uint8_t[]>(mSize * mSize * numChars);
	for (unsigned int character = 0; character < numChars; character++) {
		auto glpyhCharacter = character;
		if (character == '\t') {
			glpyhCharacter = ' ';
		}

		// Load font character glyph
		if (FT_Load_Char(face, glpyhCharacter, FT_LOAD_RENDER)) {
			std::cerr << "Failed to load glyph." << std::endl;
			continue;
		}

		if (mLineHeight == 0) {
			mLineHeight = face->glyph->metrics.vertAdvance / 64.0f;
		}

		auto width = face->glyph->bitmap.width;
		auto height = face->glyph->bitmap.rows;
		auto charMapOffset = character * mSize;

		for (std::size_t y = 0; y < height; y++) {
			for (std::size_t x = 0; x < width; x++) {
				buffer[y * (mSize * mCharsPerColumn) + charMapOffset + x] = face->glyph->bitmap.buffer[y * width + x];
			}
		}

		// Now store fontCharacter for later use
		FontCharacter fontCharacter = {
			0,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x / 64.0f,

			0.0f, // Top
			charMapOffset / maxWidth, // Left
			fontCharacter.size.y / (float)mSize, // Bottom
			(charMapOffset + fontCharacter.size.x) / maxWidth //Right
		};

		if (mMonoSpaceAdvanceX == 0.0f) {
			mMonoSpaceAdvanceX = fontCharacter.advanceX;
		}

		if (mMonoSpaceAdvanceX != fontCharacter.advanceX) {
			mMonoSpaceAdvanceX = false;
		}

		mCharacters.insert({ character, fontCharacter });
	}

	std::cout << "loaded font map" << std::endl;

	// Generate texture
	glGenTextures(1, &mTextureMap);
	glBindTexture(GL_TEXTURE_2D, mTextureMap);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		mSize * numChars,
		mSize,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		buffer.get()
	);

	// Set texture options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

Font::~Font() {
	for (auto& character : mCharacters) {
		glDeleteTextures(1, &character.second.textureId);
	}
}

GLuint Font::textureMap() const {
	return mTextureMap;
}

float Font::lineHeight() const {
	return mLineHeight;
}

void Font::getTextureCoordinates(Char character, float& top, float& left, float& bottom, float& right) const {
	auto& fontCharacter = mCharacters.at(character);
	top = fontCharacter.textureTop;
	left = fontCharacter.textureLeft;
	bottom = fontCharacter.textureBottom;
	right = fontCharacter.textureRight;
}

const FontCharacter& Font::operator[](Char character) const {
	return mCharacters.at(character);
}

float Font::getAdvanceX(Char character) const {
	if (mIsMonoSpace) {
		return mMonoSpaceAdvanceX;
	}

	return (*this)[character].advanceX;
}
