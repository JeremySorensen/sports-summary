#pragma once

#include <vector>
#include <string>
#include "Image.hpp"

Image load_jpeg(const std::vector<uint8_t>& bytes);

Image load_jpeg_from_file(const char* const filename);

AlphaImage load_png(const std::vector<uint8_t>& bytes);

AlphaImage load_png_from_file(const char* const filename);

//void save_bitmap(Image image, std::string filename);