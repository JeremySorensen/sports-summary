#pragma once

#include <vector>
#include <array>
#include "Image.hpp"
#include "load_image.hpp"
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
	Image texture;
	std::vector<int> image_ids;
	int revision = 3;

	static float float_x(int x) {
		return x / TEXTURE_WIDTH_F;
	}

	static float float_y(int y) {
		return y / TEXTURE_HEIGHT_F;
	}

public:
	ImageManager(const char* filename) : texture(load_jpeg_from_file(filename)) {
		image_ids.reserve(7);
		for (int id = 0; id < 7; ++id) {
			image_ids.push_back(id);
		}
	}

	static int get_cut_width() { return CUT_WIDTH; }
	static int get_cut_height() { return CUT_HEIGHT; }

	Image get_texture() const {
		return texture;
	}

	Bounds get_background_tex() const {
		return { float_x(0), float_x(BACKGROUND_WIDTH), float_y(BACKGROUND_HEIGHT), float_y(0) };
	}

	Bounds get_item_tex(int image_id) const {
		bool found_id = false;
		for (auto&& id : image_ids) {
			if (image_id == id) {
				found_id = true;
				break;
			}
		}

		Bounds bounds;

		if (!found_id) {
			bounds.left = float_x(0);
			bounds.right = float_x(CUT_WIDTH);
			bounds.top = float_y(BACKGROUND_HEIGHT);
			bounds.bot = float_y(BACKGROUND_HEIGHT + CUT_HEIGHT);
		}
		else {
			int row = (image_id + 2) / ROW_CUTS;
			int col = (image_id + 2) % ROW_CUTS;

			int left_int = col * CUT_WIDTH;
			int top_int = row * CUT_HEIGHT + BACKGROUND_HEIGHT;
			bounds.left = float_x(left_int);
			bounds.right = float_x(left_int + CUT_WIDTH);
			bounds.top = float_y(top_int);
			bounds.bot = float_y(top_int + CUT_HEIGHT);
		}

		return bounds;
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
