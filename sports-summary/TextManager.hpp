#pragma once

#include <vector>
#include <array>
#include "Image.hpp"
#include "load_image.hpp"
#include "Bounds.hpp"

class TextManager {
	const int ROW_HEIGHT = 26;
	const int TEXTURE_WIDTH = 512;
	const int TEXTURE_HEIGHT = 64;
	const float TEXTURE_WIDTH_F;
	const float TEXTURE_HEIGHT_F;

	// All the (fractional) pixel starts of the ASCII characters starting at
	// SPACE. It is spread accross two rows (next row starts at 'O').
	// The last value in each row is actually the pixel end of the last
	// character for that row.
	const int advances[97] = {
		0, 6, 12, 19, 30, 41, 59, 72, 76, 83, 90, 98, 110, 116, 123, 129,
		135, 146, 157, 168, 179, 190, 201, 212, 223, 234, 245, 251, 257, 269, 281, 293,
		304, 324, 337, 349, 363, 377, 390, 401, 417, 431, 437, 447, 460, 471, 488, 502,
		0, 16, 29, 45, 59, 71, 83, 97, 110, 128, 141, 153, 165, 171, 177, 183,
		192, 203, 210, 221, 232, 242, 253, 264, 270, 281, 292, 296, 300, 310, 314, 331,
		342, 353, 364, 375, 382, 392, 398, 409, 419, 433, 443, 452, 462, 469, 474, 481, 493
	};

	AlphaImage texture;

	float float_x(int x) const {
		return x / TEXTURE_WIDTH_F;
	}

	float float_y(int y) const  {
		return y / TEXTURE_HEIGHT_F;
	}

public:
	TextManager(const char* filename) : 
		texture(load_png_from_file(filename)),
		TEXTURE_HEIGHT_F(static_cast<float>(TEXTURE_HEIGHT)),
		TEXTURE_WIDTH_F(static_cast<float>(TEXTURE_WIDTH)) { }

	AlphaImage get_texture() const {
		return texture;
	}

	Bounds get_glyph_tex(char glyph) const {
		Bounds bounds;
		if (glyph >= 'O') {
			bounds.left = float_x(advances[glyph - ' ' + 1]);
			bounds.right = float_x(advances[glyph - ' ' + 2]);
			bounds.top = float_y(ROW_HEIGHT);
			bounds.bot = float_y(2 * ROW_HEIGHT);
		}
		else {
			bounds.left = float_x(advances[glyph - ' ']);
			bounds.right = float_x(advances[glyph - ' ' + 1]);
			bounds.top = float_y(0);
			bounds.bot = float_y(ROW_HEIGHT);
		}

		return bounds;
	}

	int get_glyph_width(char glyph) const {
		if (glyph >= 'O') {
			return advances[glyph - ' ' + 2] - advances[glyph - ' ' + 1];
		}
		else {
			return advances[glyph - ' ' + 1] - advances[glyph - ' '];
		}
	}

	float get_glyph_height() const { return static_cast<float>(ROW_HEIGHT); }

};
