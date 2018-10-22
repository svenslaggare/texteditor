#include "textloader.h"
#include "../helpers.h"
#include "formatters/cpp.h"
#include "formatters/python.h"
#include "formatters/text.h"

LoadedText TextLoader::load(const std::string& fileName) {
	std::unique_ptr<FormatterRules> rules;

	if (fileName.find(".cpp") != std::string::npos || fileName.find(".h") != std::string::npos) {
		rules = std::make_unique<CppFormatterRules>();
	} else if (fileName.find(".py") != std::string::npos) {
		rules = std::make_unique<PythonFormatterRules>();
	} else {
		rules = std::make_unique<TextFormatterRules>();
	}

	return { Text(Helpers::readFileAsText<String>(fileName)), std::move(rules) };
}
