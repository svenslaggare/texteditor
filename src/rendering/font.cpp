#include "font.h"
#include <iostream>
#include <memory>

namespace {
	const auto numChars = 256;

	std::vector<Char> createAllCharacters() {
		std::vector<Char> characters;
		for (Char current = 0; current < numChars; current++) {
			characters.push_back(current);
		}

		return characters;
	}
}

FontMap::FontMap(const std::string& name, std::uint32_t size, const std::vector<Char>& characters)
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
	for (auto& character : characters) {
		// Load font character glyph
		if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
			continue;
		}

		mSize = std::max(mSize, face->glyph->bitmap.width);
		mSize = std::max(mSize, face->glyph->bitmap.rows);
	}

	mCharsPerColumn = characters.size();
	const auto maxWidth = characters.size() * (float)mSize;

	static std::unordered_map<Char, Char> glpyhReplacements = {
		{ '\t', ' ' }
	};

	auto buffer = std::make_unique<std::uint8_t[]>(mSize * mSize * characters.size());
	std::size_t characterIndex = 0;
	for (auto& character : characters) {
		auto glpyhCharacter = character;

		auto replacementIterator = glpyhReplacements.find(character);
		if (replacementIterator != glpyhReplacements.end()) {
			glpyhCharacter = replacementIterator->second;
		}

		// Load font character glyph
		if (FT_Load_Char(face, glpyhCharacter, FT_LOAD_RENDER)) {
			std::cerr << "Failed to load glyph." << std::endl;
			continue;
		}

		auto width = face->glyph->bitmap.width;
		auto height = face->glyph->bitmap.rows;
		auto charMapOffset = characterIndex * mSize;

		for (std::size_t y = 0; y < height; y++) {
			for (std::size_t x = 0; x < width; x++) {
				buffer[y * (mSize * mCharsPerColumn) + charMapOffset + x] = face->glyph->bitmap.buffer[y * width + x];
			}
		}

		// Now store fontCharacter for later use
		FontCharacter fontCharacter = {
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x / 64.0f,
			face->glyph->metrics.vertAdvance / 64.0f,

			0.0f, // Top
			charMapOffset / maxWidth, // Left
			fontCharacter.size.y / (float)mSize, // Bottom
			(charMapOffset + fontCharacter.size.x) / maxWidth //Right
		};

		mCharacters.insert({ character, fontCharacter });
		characterIndex++;
	}

//	std::cout << "loaded font map" << std::endl;

	// Generate texture
	glGenTextures(1, &mTextureMap);
	glBindTexture(GL_TEXTURE_2D, mTextureMap);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		(std::uint32_t)(mSize * characters.size()),
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

FontMap::~FontMap() {
	glDeleteTextures(1, &mTextureMap);
}

GLuint FontMap::textureMap() const {
	return mTextureMap;
}

const std::unordered_map<Char, FontCharacter>& FontMap::characters() const {
	return mCharacters;
}

Font::Font(const std::string& name, std::uint32_t size)
	: mName(name), mSize(size) {
	mCreatedCharacters = createAllCharacters();
	createFontMap(mCreatedCharacters);
}

GLuint Font::textureMap() const {
	return mFontMap->textureMap();
}

float Font::lineHeight() const {
	return mLineHeight;
}

void Font::createFontMap(const std::vector<Char>& characters) {
	mFontMap = std::make_unique<FontMap>(mName, mSize, characters);

	mMonoSpaceAdvanceX = 0.0f;
	mMonoSpaceAdvanceX = false;

	for (auto& current : mFontMap->characters()) {
		auto& fontCharacter = current.second;

		if (mLineHeight == 0) {
			mLineHeight = fontCharacter.lineHeight;
		}

		if (mMonoSpaceAdvanceX == 0.0f) {
			mMonoSpaceAdvanceX = fontCharacter.advanceX;
		}

		if (mMonoSpaceAdvanceX != fontCharacter.advanceX) {
			mMonoSpaceAdvanceX = false;
		}
	}

	std::cout << "Created font map" << std::endl;
}

const FontCharacter& Font::operator[](Char character) const {
	return mFontMap->characters().at(character);
}

const FontCharacter& Font::tryGet(Char character) {
	auto charIterator = mFontMap->characters().find(character);
	if (charIterator != mFontMap->characters().end()) {
		return charIterator->second;
	} else {
		mCreatedCharacters.push_back(character);
		createFontMap(mCreatedCharacters);
		return mFontMap->characters().at(character);
	}
}

float Font::getAdvanceX(Char character) const {
	if (mIsMonoSpace) {
		return mMonoSpaceAdvanceX;
	}

	return (*this)[character].advanceX;
}
