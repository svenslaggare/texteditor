#pragma once
#include <memory>
#include "text.h"

class FormatterRules;

struct LoadedText {
	Text text;
	std::unique_ptr<FormatterRules> rules;
};

/**
 * Represents a text loader
 */
class TextLoader {
public:
	/**
	 * Loads the text from the given filename
	 * @param fileName The file name
	 * @return
	 */
	LoadedText load(const std::string& fileName);
};