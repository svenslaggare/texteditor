#pragma once
#include <vector>
#include <functional>
#include <iostream>
#include <unordered_set>
#include "../external/tsl/array_set.h"
#include "text.h"

/**
 * Represents a circular buffer
 * @tparam T
 */
template<typename T>
class CircularBuffer {
private:
	std::vector<T> mData;
	std::size_t mMaxSize;
	std::size_t mCurrentIndex = 0;
public:
	/**
	 * Creates a new buffer with the given size
	 * @param maxSize The max size
	 */
	explicit CircularBuffer(std::size_t maxSize);

	/**
	 * Adds the given value to the buffer
	 * @param value The value
	 */
	void add(const T& value);

	/**
	 * Applies the given function to each element
	 * @param apply The function
	 * @param maxSize The max size
	 */
	void forEachElement(std::function<void (const T&)> apply, std::size_t maxSize = (std::size_t)(-1L));
};

template<typename T>
CircularBuffer<T>::CircularBuffer(std::size_t maxSize)
	: mMaxSize(maxSize) {

}

template<typename T>
void CircularBuffer<T>::add(const T& value) {
	if (mData.size() < mMaxSize) {
		mData.push_back(value);
		mCurrentIndex = (mCurrentIndex + 1) % mMaxSize;
	} else {
		mData[mCurrentIndex] = value;
		mCurrentIndex = (mCurrentIndex + 1) % mMaxSize;
	}
}

template<typename T>
void CircularBuffer<T>::forEachElement(std::function<void(const T&)> apply, std::size_t maxSize) {
	bool showAll = maxSize == (std::size_t)(-1L);
	for (std::size_t i = 0; i < mData.size(); i++) {
		auto realIndex = (i + mCurrentIndex) % mData.size();
		if (showAll || i + maxSize >= mData.size()) {
			apply(mData[realIndex]);
		}
	}
}

/**
 * Represents a keyword list
 */
class KeywordList {
private:
	tsl::array_set<Char> mKeywords;
	std::size_t mMaxLength = 0;
public:
	/**
	 * Creates a new keyword list
	 * @param keywords The keywords
	 */
	explicit KeywordList(const std::unordered_set<std::string>& keywords);

	/**
	 * Indicates if the given string is a keyword
	 * @param str The string
	 * @return
	 */
	inline bool isKeyword(const String& str) const {
		if (str.size() <= mMaxLength) {
			return mKeywords.count(str) > 0;
		}

		return false;
	}
};