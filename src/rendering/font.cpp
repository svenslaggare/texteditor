#include "font.h"
#include <iostream>
#include <memory>

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

	const auto numChars = 128;

	auto buffer = new std::uint8_t[size * size * numChars] { 0 };
	for (GLubyte character = 0; character < numChars; character++) {
		auto glpyhCharacter = character;
		if (character == '\t') {
			glpyhCharacter = ' ';
		}

		// Load fontCharacter glyph
		if (FT_Load_Char(face, glpyhCharacter, FT_LOAD_RENDER)) {
			std::cout << "Failed to load glyph." << std::endl;
			continue;
		}

		if (mLineHeight == 0) {
			mLineHeight = face->glyph->metrics.vertAdvance / 64.0f;
		}

		auto width = face->glyph->bitmap.width;
		auto height = face->glyph->bitmap.rows;
		for (std::size_t y = 0; y < height; y++) {
			for (std::size_t x = 0; x < width; x++) {
				buffer[y * (size * numChars) + (character * size) + x] = face->glyph->bitmap.buffer[y * width + x];
			}
		}

		// Now store fontCharacter for later use
		FontCharacter fontCharacter = {
			0,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x / 64.0f
		};

		mCharacters.insert({ character, fontCharacter });
	}

	// Generate texture
	glGenTextures(1, &mTextureMap);
	glBindTexture(GL_TEXTURE_2D, mTextureMap);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		size * numChars,
		size,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		buffer
	);

	// Set texture options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete[] buffer;

//	for (GLubyte c = 0; c < numChars; c++) {
//		// Load character glyph
//		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
//			std::cout << "Failed to load glyph." << std::endl;
//			continue;
//		}
//
//		// Generate texture
//		GLuint texture;
//		glGenTextures(1, &texture);
//		glBindTexture(GL_TEXTURE_2D, texture);
//		glTexImage2D(
//			GL_TEXTURE_2D,
//			0,
//			GL_RED,
//			face->glyph->bitmap.width,
//			face->glyph->bitmap.rows,
//			0,
//			GL_RED,
//			GL_UNSIGNED_BYTE,
//			face->glyph->bitmap.buffer
//		);
//
//		// Set texture options
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//		// Now store character for later use
//		FontCharacter character = {
//			texture,
//			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//			face->glyph->advance.x / 64.0f
//		};
//
//		mCharacters.insert({ c, character });
//	}

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

void Font::getTextureCoordinates(char character, float& top, float& left, float& bottom, float& right) const {
	const auto numChars = 128;
	const auto maxWidth = numChars * (float)mSize;

	auto& fontCharacter = mCharacters.at(character);

	top = 0.0f;
	left = (character * mSize) / maxWidth;
	bottom = fontCharacter.size.y / (float)mSize;
	right = (character * mSize + fontCharacter.size.x) / maxWidth;
}

const FontCharacter& Font::operator[](char character) const {
	return mCharacters.at(character);
}
