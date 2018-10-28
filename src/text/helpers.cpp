#include "helpers.h"
#include "../helpers.h"

KeywordList::KeywordList(const std::unordered_set<std::string>& keywords) {
	for (auto& keyword : keywords) {
		auto keywordStr = Helpers::fromString<String>(keyword);
		mKeywords.insert(keywordStr);
		mMaxLength = std::max(mMaxLength, keyword.size());
	}
}