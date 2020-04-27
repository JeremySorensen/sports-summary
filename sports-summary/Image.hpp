// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// These structures store image data and size, either with and alpha channel
// or without.

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
