#include <vector>
#include <gsl/gsl>
#include "gl.h"
#include "build_shaders.hpp"
#include "load_texture.hpp"
#include "errors.hpp"
#include "Display.hpp"

using std::vector;
;
const int FLOATS_PER_QUAD = 16; // 4 x,y points + 4 s,t points
const int QUADS_IN_FRAME = 8;
const int INDICES_PER_QUAD = 6;
const int PAD = 16;
const int FRAME_WIDTH = 8;
const int FRAME_PAD = 4;
const int TEXT_PAD = 4;

const int EXTRA_SELECTED_WIDTH = 2 * (FRAME_WIDTH + FRAME_PAD);

Display::Display(
	const ImageManager& image_manager,
	const TextManager& text_manager,
	std::vector<DisplayItem> items,
	int width,
	int height):
    image_manager(image_manager), text_manager(text_manager), items(items), width(width), height(height) {

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		ui_error("Error loading OpenGL", __FILE__, __LINE__);
	}

	shader_program = build_shaders("shaders/vertex.glsl", "shaders/fragment.glsl");

	glViewport(0, 0, width, height);

	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);
	glGenBuffers(2, ebo);
	glBindVertexArray(vao);

	glGenTextures(2, texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto texture = text_manager.get_texture();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bytes.data());

	glBindTexture(GL_TEXTURE_2D, texture_id[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Display::update_geometry(int selected_item_id) {
	int selected_item_width = ImageManager::get_cut_width();
	int selected_item_height = ImageManager::get_cut_height();

	float item_width = selected_item_width / 1.5f;
	float item_height = selected_item_height / 1.5f;

	int num_items = items.size();
	int width_without_selected = width - selected_item_width - EXTRA_SELECTED_WIDTH - PAD;
	int num_unselected_items = 1 + width_without_selected / (item_width + PAD);
	if (num_unselected_items >= num_items - 1) { num_unselected_items = num_items - 1; }

	// The +1 is to draw the item that is partially off the screen
	int first_id = selected_item_id - num_unselected_items + 1;
	if (first_id < 0) { first_id = 0; }
	int selected_index = selected_item_id - first_id;
	int num_after_selected = num_unselected_items - selected_index;
	// Because we draw a partial extra item, we need to do one less when
	// the selected item is all the way to the right
	if (selected_item_id >= num_items - 1) {
		num_after_selected = 0;
	}

	int num_headline_rects = items[selected_item_id].headline.size();
	int num_long_text_rects = items[selected_item_id].text.size();
	int num_text_rects = num_headline_rects + num_long_text_rects;
	int num_text_floats = num_headline_rects * FLOATS_PER_QUAD;
	text_floats.clear();
	if (num_text_floats > text_floats.size()) {
		text_floats.reserve(num_text_floats);
		text_indices.reserve(num_text_rects * INDICES_PER_QUAD);
	}

	int num_rects = (num_unselected_items + 2 + QUADS_IN_FRAME);
	int num_floats = num_rects * FLOATS_PER_QUAD;

	floats.clear();
	indices.clear();

	if (num_floats > floats.size()) {
		floats.reserve(num_floats);
		indices.reserve(num_rects * INDICES_PER_QUAD);
	}

	int index = add_quad(0, { -1.0f, 1.0f, -1.0f, 1.0f }, image_manager.get_background_tex());
	float top_px = (height + item_height) / 2;
	float bot_px = top_px - item_height;
	float top = float_y(top_px);
	float bot = float_y(bot_px);

	// All the items before the selected item
	for (int i = 0; i < selected_index; ++i) {
		int left_px = i * (item_width + PAD) + PAD;
		int right_px = left_px + item_width;
		float left = float_x(left_px);
		float right = float_x(right_px);

		auto tex = image_manager.get_item_tex(items[i + first_id].image_id);

		index = add_quad(index, { left, right, top, bot }, tex);
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
	index = add_quad(
		index,
		{ frame_in_right, frame_out_right, frame_out_top, frame_in_top },
		frame_right_top_tex
	);

	// bottom right frame corner
	index = add_quad(
		index,
		{ frame_in_right, frame_out_right, frame_in_bot, frame_out_bot },
		frame_right_bot_tex
	);

	// bottom left frame corner
	index = add_quad(
		index,
		{ frame_out_left, frame_in_left, frame_in_bot, frame_out_bot },
		frame_left_bot_tex
	);

	// top left frame corner
	index = add_quad(
		index,
		{ frame_out_left, frame_in_left, frame_out_top, frame_in_top },
		frame_left_top_tex
	);

	// left frame
	index = add_quad(
		index,
		{ frame_out_left, frame_in_left, frame_in_top, frame_in_bot },
		frame_vert_tex
	);

	// right frame
	index = add_quad(
		index,
		{ frame_in_right, frame_out_right, frame_in_top, frame_in_bot },
		frame_vert_tex
	);

	// top frame
	index = add_quad(
		index,
		{ frame_in_left, frame_in_right, frame_out_top, frame_in_top },
		frame_horz_tex
	);

	// bot frame
	index = add_quad(
		index,
		{ frame_in_left, frame_in_right, frame_in_bot, frame_out_bot },
		frame_horz_tex
	);

	// selected image
	index = add_quad(
		index,
		{ selected_left, selected_right, selected_top, selected_bot },
		selected_tex
	);

	// The items after the selected item
	for (int i = 0; i < num_after_selected; ++i) {
		int left_px = i * (item_width + PAD) + frame_out_right_px + PAD;
		int right_px = left_px + item_width;
		float left = float_x(left_px);
		float right = float_x(right_px);

		auto tex = image_manager.get_item_tex(items[i + selected_item_id + 1].image_id);

		index = add_quad(
			index,
			{ left, right, top, bot },
			tex
		);
	}

	// Headline text

	int headline_bot_px = frame_out_top_px + TEXT_PAD;
	float headline_top_px = headline_bot_px + text_manager.get_glyph_height();
	float headline_bot = float_y(headline_bot_px);
	float headline_top = float_y(headline_top_px);
	float glyph_left_px = frame_out_left_px;
	index = 0;
	for (auto&& glyph : items[selected_item_id].headline) { 
		float glyph_right_px = glyph_left_px + text_manager.get_glyph_width(glyph);
		index = add_text_quad(
			index,
			{ float_x(glyph_left_px), float_x(glyph_right_px), headline_top, headline_bot },
			text_manager.get_glyph_tex(glyph)
		);
		glyph_left_px = glyph_right_px;
	}

	// Longer text
	int text_top_px = frame_out_bot_px - TEXT_PAD;
	float text_bot_px = text_top_px - text_manager.get_glyph_height();
	float text_top = float_y(text_top_px);
	float text_bot = float_y(text_bot_px);
	glyph_left_px = frame_out_left_px;
	index = 0;
	for (auto&& glyph : items[selected_item_id].text) {
		float glyph_right_px = glyph_left_px + text_manager.get_glyph_width(glyph);
		index = add_text_quad(
			index,
			{ float_x(glyph_left_px), float_x(glyph_right_px), text_top, text_bot },
			text_manager.get_glyph_tex(glyph)
			);
		glyph_left_px = glyph_right_px;
	}
}

void Display::draw(int selected_item_id) {

	if (image_manager.get_revision() != texture_revision) {
		texture_revision = image_manager.get_revision();

		auto texture = image_manager.get_texture();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.bytes.data());
	}

	update_geometry(selected_item_id);

	// Draw geometry
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, floats.size() * sizeof(float), floats.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindTexture(GL_TEXTURE_2D, texture_id[1]);

	glUseProgram(shader_program);
	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

	// Draw Text
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, text_floats.size() * sizeof(float), text_floats.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, text_indices.size() * sizeof(float), text_indices.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindTexture(GL_TEXTURE_2D, texture_id[0]);

	glUseProgram(shader_program);
	//glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, text_indices.size(), GL_UNSIGNED_INT, nullptr);
}