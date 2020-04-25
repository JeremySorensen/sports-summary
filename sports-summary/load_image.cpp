#include <fstream>
#include <iostream>
#include <gsl/gsl>
#include <cstring>
#include "load_image.hpp"
#include "utils.hpp"
#include "errors.hpp"

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::vector;
using std::memcpy;

Image load_jpeg(const vector<uint8_t>& bytes) {

	int width = 0;
	int height = 0;
	int comp;

	auto ok = stbi_info_from_memory(bytes.data(), bytes.size(), &width, &height, &comp);

	if (ok == 0) {
		lib_error("failed to get JPEG header info", __FILE__, __LINE__);
	}

	Image image(width, height);

	// add scope for auto-cleanup
	{
		stbi_uc* data = stbi_load_from_memory(bytes.data(), bytes.size(), &width, &height, &comp, 0);
		if (data == nullptr) {
			lib_error("Error decompressing JPEG", __FILE__, __LINE__);
		}
		auto free_image = gsl::finally([data] { stbi_image_free(data); });
		memcpy_s(image.bytes.data(), image.bytes.size(), data, width * height * 3);
	}

	return image;
}

Image load_jpeg_from_file(const char* const filename) {
	return load_jpeg(read_all_bytes(filename));
}

AlphaImage load_png(const vector<uint8_t>& bytes) {

	int width = 0;
	int height = 0;
	int comp;

	auto ok = stbi_info_from_memory(bytes.data(), bytes.size(), &width, &height, &comp);

	if (ok == 0) {
		lib_error("failed to get PNG header info", __FILE__, __LINE__);
	}

	AlphaImage image(width, height);

	// add scope for auto-cleanup
	{
		stbi_uc* data = stbi_load_from_memory(bytes.data(), bytes.size(), &width, &height, &comp, 0);
		if (data == nullptr) {
			lib_error("Error decompressing PNGr", __FILE__, __LINE__);
		}
		auto free_image = gsl::finally([data] { stbi_image_free(data); });
		memcpy_s(image.bytes.data(), image.bytes.size(), data, width * height * 4);
	}

	return image;
}

AlphaImage load_png_from_file(const char* filename) {
	return load_png(read_all_bytes(filename));
}