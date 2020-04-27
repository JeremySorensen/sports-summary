// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// Functions to load JPEG and PNG images from files or bytes.
// Can also uncomment the declaration here and definition in the .cpp file to
// have a function to save bitmaps for debugging.
// The JPEG and PNG loaders use stb_image to do the work.

#pragma once

#include <vector>
#include <string>
#include "Image.hpp"

Image load_jpeg(const std::vector<uint8_t>& bytes);

Image load_jpeg_from_file(const char* const filename);

AlphaImage load_png(const std::vector<uint8_t>& bytes);

AlphaImage load_png_from_file(const char* const filename);

//void save_bitmap(Image image, std::string filename);