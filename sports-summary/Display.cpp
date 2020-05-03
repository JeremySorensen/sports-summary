// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// See header file for more information.

#include <vector>
#include <gsl/gsl>
#include "load_texture.hpp"
#include "errors.hpp"
#include "Display.hpp"

using std::vector;

const int FLOATS_PER_POINT = 5; // x, y, s, t, texture
const int FLOATS_PER_QUAD = 4 * FLOATS_PER_POINT;
const int QUADS_IN_FRAME = 8; // 4 sides, 4 corners
const int INDICES_PER_QUAD = 6; // 2 triangles & 3 points (indices) per tri
const int PAD = 16; // pixel padding between items
const int FRAME_WIDTH = 8; // pixel width of frame sides
const int FRAME_PAD = 4; // space between frame and selected item
const int TEXT_PAD = 4; // space between text and frame

const int EXTRA_SELECTED_WIDTH = 2 * (FRAME_WIDTH + FRAME_PAD);

Display::Display(
	OpenGlDrawer& drawer,
	TextManager& text_manager,
	ImageManager& image_manager,
	int width,
	int height):
    drawer(drawer), text_manager(text_manager), image_manager(image_manager), width(width), height(height) {

	items.push_back({ 0, "Getting your scores", "", 0 });

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		ui_error("Error loading OpenGL", __FILE__, __LINE__);
	}
}

Display::TextLocations Display::update_geometry(int selected_item_id) {
	int selected_item_width = ImageManager::get_cut_width();
	int selected_item_height = ImageManager::get_cut_height();

	float item_width = selected_item_width / 1.5f;
	float item_height = selected_item_height / 1.5f;

	int num_items = items.size();
	int width_without_selected = width - selected_item_width - EXTRA_SELECTED_WIDTH - PAD;
	int num_unselected_items = 1 + static_cast<int>(width_without_selected / (item_width + PAD));
	if (num_unselected_items >= num_items - 1) { num_unselected_items = num_items - 1; }

	// The +1 is to draw the item that is partially off the screen
	int first_id = selected_item_id - num_unselected_items + (num_unselected_items == 0 ? 0 : 1);
	if (first_id < 0) { first_id = 0; }
	int selected_index = selected_item_id - first_id;
	int num_after_selected = num_unselected_items - selected_index;
	// Because we draw a partial extra item, we need to do one less when
	// the selected item is all the way to the right
	if (selected_item_id >= num_items - (num_unselected_items == 0 ? 0 : 1)) {
		num_after_selected = 0;
	}

	size_t num_headline_rects = items[selected_item_id].headline.size();
	size_t num_long_text_rects = items[selected_item_id].text.size();
	size_t num_text_rects = num_headline_rects + num_long_text_rects;
	size_t num_text_floats = num_text_rects * FLOATS_PER_QUAD;


	add_quad({ -1.0f, 1.0f, -1.0f, 1.0f }, image_manager.get_background_tex(), 0);
	float top_px = (height + item_height) / 2;
	float bot_px = top_px - item_height;
	float top = float_y(top_px);
	float bot = float_y(bot_px);

	// All the items before the selected item
	for (int i = 0; i < selected_index; ++i) {
		float left_px = i * (item_width + PAD) + PAD;
		float right_px = left_px + item_width;
		float left = float_x(left_px);
		float right = float_x(right_px);

		auto tex = image_manager.get_item_tex(items[i + first_id].image_id);

		add_quad({ left, right, top, bot }, tex, 0);
	}

	// The selected item and frame

	// frame_out_left   item_right
	// | frame_in_left | frame_in_right
	// | | item_left   | | frame_out_right
	// | | |           | | |
	// HEADLINE.............
	// +-+-+-----------+-+-+-- frame_out_top
	// |f|f|  frame    |f|f|
	// +-+-+-----------+-+-+-- frame_in_top
	// |f|p|  pad      |p|f|
	// +-+-+-----------+-+-+-- item_top
	// | | |           | | |
	// |f|p|   ITEM    |p|f|
	// | | |           | | |
	// +-+-+-----------+-+-+-- item_bot
	// |f|p|  pad      |p|f|
	// +-+-+-----------+-+-+-- frame_in_bot
	// |f|f|  frame    |f|f|
	// +-+-+-----------+--++-- frame_out_bt
	// TEXT.................
	// .....................

	auto frame_right_top_tex = image_manager.get_frame_right_top_tex();
	auto frame_right_bot_tex = image_manager.get_frame_right_bot_tex();
	auto frame_left_bot_tex = image_manager.get_frame_left_bot_tex();
	auto frame_left_top_tex = image_manager.get_frame_left_top_tex();
	auto frame_vert_tex = image_manager.get_frame_vert_side_tex();
	auto frame_horz_tex = image_manager.get_frame_horz_side_tex();
	auto selected_tex = image_manager.get_item_tex(items[selected_item_id].image_id);

	float frame_out_left_px = selected_index * (item_width + PAD) + PAD;
	float frame_in_left_px = frame_out_left_px + FRAME_WIDTH;
	float selected_left_px = frame_in_left_px + FRAME_PAD;
	float selected_right_px = selected_left_px + selected_item_width;
	float frame_in_right_px = selected_right_px + FRAME_PAD;
	float frame_out_right_px = frame_in_right_px + FRAME_WIDTH;

	float frame_out_left = float_x(frame_out_left_px);
	float frame_in_left = float_x(frame_in_left_px);
	float selected_left = float_x(selected_left_px);
	float selected_right = float_x(selected_right_px);
	float frame_in_right = float_x(frame_in_right_px);
	float frame_out_right = float_x(frame_out_right_px);

	int selected_top_px = (height + selected_item_height) / 2;
	int frame_in_top_px = selected_top_px + FRAME_PAD;
	int frame_out_top_px = frame_in_top_px + FRAME_WIDTH;
	int selected_bot_px = selected_top_px - selected_item_height;
	int frame_in_bot_px = selected_bot_px - FRAME_PAD;
	int frame_out_bot_px = frame_in_bot_px - FRAME_WIDTH;

	float selected_top = float_y(selected_top_px);
	float frame_in_top = float_y(frame_in_top_px);
	float frame_out_top = float_y(frame_out_top_px);
	float selected_bot = float_y(selected_bot_px);
	float frame_in_bot = float_y(frame_in_bot_px);
	float frame_out_bot = float_y(frame_out_bot_px);

	// top right frame corner
	add_quad(
		{ frame_in_right, frame_out_right, frame_out_top, frame_in_top },
		frame_right_top_tex,
		0
	);

	// bottom right frame corner
	add_quad(
		{ frame_in_right, frame_out_right, frame_in_bot, frame_out_bot },
		frame_right_bot_tex,
		0
	);

	// bottom left frame corner
	add_quad(
		{ frame_out_left, frame_in_left, frame_in_bot, frame_out_bot },
		frame_left_bot_tex,
		0
	);

	// top left frame corner
	add_quad(
		{ frame_out_left, frame_in_left, frame_out_top, frame_in_top },
		frame_left_top_tex,
		0
	);

	// left frame
	add_quad(
		{ frame_out_left, frame_in_left, frame_in_top, frame_in_bot },
		frame_vert_tex,
		0
	);

	// right frame
	add_quad(
		{ frame_in_right, frame_out_right, frame_in_top, frame_in_bot },
		frame_vert_tex,
		0
	);

	// top frame
	add_quad(
		{ frame_in_left, frame_in_right, frame_out_top, frame_in_top },
		frame_horz_tex,
		0
	);

	// bot frame
	add_quad(
		{ frame_in_left, frame_in_right, frame_in_bot, frame_out_bot },
		frame_horz_tex,
		0
	);

	// selected image
	add_quad(
		{ selected_left, selected_right, selected_top, selected_bot },
		selected_tex,
		0
	);

	// The items after the selected item
	for (int i = 0; i < num_after_selected; ++i) {
		float left_px = i * (item_width + PAD) + frame_out_right_px + PAD;
		float right_px = left_px + item_width;
		float left = float_x(left_px);
		float right = float_x(right_px);

		auto tex = image_manager.get_item_tex(items[i + selected_item_id + 1].image_id);

		add_quad({ left, right, top, bot }, tex, 0);
	}

	return TextLocations{ frame_out_left_px, frame_out_top_px, frame_out_bot_px };
}

void Display::update_text(int selected_item_id, TextLocations locations) {
		
	// Headline text

	int headline_bot_px = locations.top_text_y + TEXT_PAD;
	float headline_top_px = headline_bot_px + text_manager.get_glyph_height();
	float headline_bot = float_y(headline_bot_px);
	float headline_top = float_y(headline_top_px);
	float glyph_left_px = locations.text_x;
	int index = 0;
	for (auto&& glyph : items[selected_item_id].headline) { 
		float glyph_right_px = glyph_left_px + text_manager.get_glyph_width(glyph);
		add_quad(
			{ float_x(glyph_left_px), float_x(glyph_right_px), headline_top, headline_bot },
			text_manager.get_glyph_tex(glyph),
			1
		);
		glyph_left_px = glyph_right_px;
	}

	// Blurb text
	int text_top_px = locations.bot_text_y - TEXT_PAD;
	float text_bot_px = text_top_px - text_manager.get_glyph_height();
	float text_top = float_y(text_top_px);
	float text_bot = float_y(text_bot_px);
	glyph_left_px = locations.text_x;
	for (auto&& glyph : items[selected_item_id].text) {
		float glyph_right_px = glyph_left_px + text_manager.get_glyph_width(glyph);
		add_quad(
			{ float_x(glyph_left_px), float_x(glyph_right_px), text_top, text_bot },
			text_manager.get_glyph_tex(glyph),
			1
		);
		glyph_left_px = glyph_right_px;
	}
}

void Display::update_indices() {
	//indices.clear();
	//auto num_points = floats.size() / FLOATS_PER_POINT;
	//for (size_t index = 0; index < num_points; index += 4) {
	//	indices.push_back(index + 0);
	//	indices.push_back(index + 1);
	//	indices.push_back(index + 3);
	//	indices.push_back(index + 1);
	//	indices.push_back(index + 2);
	//	indices.push_back(index + 3);
	//}

	// above code clears indices and re-adds all of them, below code
	// adds any needed, or removes any extra instead.

	auto num_points = floats.size() / FLOATS_PER_POINT;

	auto num_indexed_quads = indices.size() / INDICES_PER_QUAD;
	auto num_indexed_floats = num_indexed_quads * FLOATS_PER_QUAD;
	
	if (num_indexed_floats < floats.size()) {
		// add indices
		int start_index = num_indexed_quads * 4; // 4 points per quad
		for (int index = start_index; index < num_points; index += 4) {
			indices.push_back(index + 0);
			indices.push_back(index + 1);
			indices.push_back(index + 3);
			indices.push_back(index + 1);
			indices.push_back(index + 2);
			indices.push_back(index + 3);
		}
	}
	else if (num_indexed_floats > floats.size()) {
	    // remove extra indices
		auto num_indices = floats.size() / FLOATS_PER_QUAD * INDICES_PER_QUAD;
		indices.resize(num_indices);
	}
	else {
		// have correct number of indices, do nothing
	}
}

void Display::draw(int selected_item_id) {

	floats.clear();
	auto locations = update_geometry(selected_item_id);
	update_text(selected_item_id, locations);
	update_indices();

	drawer.draw(floats, indices, image_manager);
}