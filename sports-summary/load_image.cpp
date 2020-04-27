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

static void push_byte(vector<uint8_t>& data, uint8_t value) {
	data.push_back(value);
}

static void push_short(vector<uint8_t>& data, uint16_t value) {
	auto ptr = reinterpret_cast<uint8_t*>(&value);
	data.push_back(ptr[0]);
	data.push_back(ptr[1]);
}

static void push_int(vector<uint8_t>& data, uint32_t value) {
	auto ptr = reinterpret_cast<uint8_t*>(&value);
	data.push_back(ptr[0]);
	data.push_back(ptr[1]);
	data.push_back(ptr[2]);
	data.push_back(ptr[3]);
}

//void save_bitmap(Image image, std::string filename) {
//	vector<uint8_t> data;
//	push_byte(data, 'B');
//	push_byte(data, 'M');
//	push_int(data, 54 + image.bytes.size()); // file size
//	push_int(data, 0); // two reserved shorts
//	push_int(data, 54); // offset to data
//	push_int(data, 40); // header size
//	push_int(data, image.width); // width
//	push_int(data, image.height); // height
//	push_short(data, 1); // planes
//	push_short(data, 24); // bits per pixel
//	push_int(data, 0); // compression
//	push_int(data, 0); // unused size
//	push_int(data, 0); // unused x px/meter
//	push_int(data, 0); // unused y px/meter
//	push_int(data, 0); // unused colors used
//	push_int(data, 0); // important colors (all)
//
//	for (int i = 0; i < image.width * image.height * 3; i += 3) {
//		data.push_back(image.bytes[i + 2]);
//		data.push_back(image.bytes[i + 1]);
//		data.push_back(image.bytes[i + 0]);
//	}
//
//	std::ofstream out_file(filename, std::ios::binary);
//	out_file.write(reinterpret_cast<char*>(data.data()), data.size());
//}