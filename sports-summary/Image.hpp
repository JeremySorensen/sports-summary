#pragma once

#include <vector>

struct Image {
	int width;
	int height;
	std::vector<uint8_t> bytes;

	Image(int width, int height) : width(width), height(height) {
		bytes.resize(size_t(3) * width * height);
	}
};

struct AlphaImage {
	int width;
	int height;
	std::vector<uint8_t> bytes;

	AlphaImage(int width, int height) : width(width), height(height) {
		bytes.resize(size_t(4) * width * height);
	}
};
