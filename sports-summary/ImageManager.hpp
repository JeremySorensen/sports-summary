// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// This class manages the large texture that holds the background, frame
// texture and all the 'cuts' which are the images for each game.
// Inserting the cuts delegated to the TextureManager, but this class provides
// the texture coordinates for all the non-text geometry.

#pragma once

#include <vector>
#include <array>
#include "Image.hpp"
#include "load_image.hpp"
#include "TextureManager.hpp"
#include "Bounds.hpp"

const int TEXTURE_WIDTH = 2048;
const int TEXTURE_HEIGHT = 2048;
const float TEXTURE_WIDTH_F = 2048.0f;
const float TEXTURE_HEIGHT_F = 2048.0f;
const int BACKGROUND_WIDTH = 1920;
const int BACKGROUND_HEIGHT = 1080;
const int CUT_WIDTH = 320;
const int CUT_HEIGHT = 180;
const int ROW_CUTS = TEXTURE_WIDTH / CUT_WIDTH;
const int FRAME_TEX_SIZE = 32;

class ImageManager {
	TextureManager texture_manager;
	std::vector<int> image_ids;
	int revision = 0;

	static float float_x(float x) {
		return x / TEXTURE_WIDTH_F;
	}

	static float float_y(float y) {
		return y / TEXTURE_HEIGHT_F;
	}

public:
	ImageManager(const char* filename) : 
		texture_manager(
			load_jpeg_from_file(filename),
			TEXTURE_WIDTH,
			TEXTURE_HEIGHT,
			2 * CUT_WIDTH,
			BACKGROUND_HEIGHT) {
	}

	static int get_cut_width() { return CUT_WIDTH; }
	static int get_cut_height() { return CUT_HEIGHT; }

	void set_max_images(int max_images) { texture_manager.set_max_images(max_images); }

	void add_jpeg(int id, const std::vector<uint8_t>& raw_bytes) {
		auto image = load_jpeg(raw_bytes);
		texture_manager.add_image(id, image);
		++revision;
	}

	void add_image(int id, Image image) {
		texture_manager.add_image(id, image);
		++revision;
	}

	Image get_texture() const {
		return texture_manager.get_texture();
	}

	Bounds get_background_tex() const {
		return { float_x(0), float_x(BACKGROUND_WIDTH), float_y(BACKGROUND_HEIGHT), float_y(0) };
	}

	Bounds get_item_tex(int image_id) const {
		auto bounds_opt = texture_manager.get_bounds(image_id);

		if (!bounds_opt) {
			return { 
				float_x(0), 
				float_x(CUT_WIDTH), 
				float_y(BACKGROUND_HEIGHT),
				float_y(BACKGROUND_HEIGHT + CUT_HEIGHT)
			};
		}
		else {
			auto bounds = bounds_opt.value();
			return {
				float_x(bounds.left),
				float_x(bounds.right),
				float_y(bounds.top),
				float_y(bounds.bot),
			};
		}
	}

	Bounds get_frame_vert_side_tex() const {
		return {
			float_x(BACKGROUND_WIDTH),
			float_x(BACKGROUND_WIDTH + FRAME_TEX_SIZE),
			float_y(1),
			float_y(2)
		};
	}

	Bounds get_frame_horz_side_tex() const {
		return {
			float_x(BACKGROUND_WIDTH + FRAME_TEX_SIZE + 1),
			float_x(BACKGROUND_WIDTH + FRAME_TEX_SIZE + 2),
			float_y(0),
			float_y(FRAME_TEX_SIZE)
		};
	}

	Bounds get_frame_right_top_tex() const {
		return {
			float_x(BACKGROUND_WIDTH + 2 * FRAME_TEX_SIZE),
			float_x(BACKGROUND_WIDTH + 3 * FRAME_TEX_SIZE),
			float_y(FRAME_TEX_SIZE),
			float_y(0),
		};
	}

	Bounds get_frame_right_bot_tex() const {
		return {
			float_x(BACKGROUND_WIDTH + 2 * FRAME_TEX_SIZE),
			float_x(BACKGROUND_WIDTH + 3 * FRAME_TEX_SIZE),
			float_y(0),
			float_y(FRAME_TEX_SIZE),
		};
	}

	Bounds get_frame_left_bot_tex() const {
		return {
			float_x(BACKGROUND_WIDTH + 3 * FRAME_TEX_SIZE),
			float_x(BACKGROUND_WIDTH + 2 * FRAME_TEX_SIZE),
			float_y(0),
			float_y(FRAME_TEX_SIZE),
		};
	}

	Bounds get_frame_left_top_tex() const {
		return {
			float_x(BACKGROUND_WIDTH + 3 * FRAME_TEX_SIZE),
			float_x(BACKGROUND_WIDTH + 2 * FRAME_TEX_SIZE),
			float_y(FRAME_TEX_SIZE),
			float_y(0),
		};
	}

	int get_revision() const { return revision; }
};
