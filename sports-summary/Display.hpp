// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// The Display class handles figuring out the vertices and texture coordinates
// for all the geometry.
// The word geometry is misused a little to mean non-text geometry.
// Everything is a quad.
// Each floating point value (2 per vertex) for the (non-text) geometry is
// stored int 'floats', the indices are stored in 'indices'. Each floating
// point value for the text quads is stored in 'text_floats' and the indices in
// 'text_indices'. This class handles the actual drawing by delegating to the
// OpenGlDrawer.


#pragma once

#include <vector>
#include <functional>
#include "Image.hpp"
#include "DisplayItem.hpp"
#include "Bounds.hpp"
#include "TextManager.hpp"
#include "ImageManager.hpp"
#include "OpenGlDrawer.hpp"

class Display {

	struct TextLocations {
		float text_x;
		int top_text_y;
		int bot_text_y;
	};

	OpenGlDrawer& drawer;
	TextManager& text_manager;
	ImageManager& image_manager;
	std::vector<DisplayItem> items;
	std::vector<float> floats;
	std::vector<unsigned int> indices;
	std::vector<float> text_floats;
	std::vector<unsigned int> text_indices;
	int width;
	int height;

	TextLocations update_geometry(int selected_item);

	void update_text(int selected_item, TextLocations locations);

	void update_indices();

	float float_x(float x) {
		return x / (width / 2) - 1.0f;
	}

	float float_y(float y) {
		return y / (height / 2) - 1.0f;
	}

	float float_x(int x) {
		return float(x) / (width / 2) - 1.0f;
	}

	float float_y(int y) {
		return float(y) / (height / 2) - 1.0f;
	}

	void add_point(float x, float y, float s, float t, int texture) {
		floats.push_back(x);
		floats.push_back(y);
		floats.push_back(s);
		floats.push_back(t);
		floats.push_back(texture);
	}

	void add_quad(const Bounds& bounds, const Bounds& tex_bounds, int texture) {
		add_point(bounds.right, bounds.top, tex_bounds.right, tex_bounds.top, texture);
		add_point(bounds.right, bounds.bot, tex_bounds.right, tex_bounds.bot, texture);
		add_point(bounds.left, bounds.bot, tex_bounds.left, tex_bounds.bot, texture);
		add_point(bounds.left, bounds.top, tex_bounds.left, tex_bounds.top, texture);
	}

public:
	Display(
		OpenGlDrawer& drawer,
		TextManager& text_manager,
		ImageManager& image_manager,
		int width,
		int height);

	void set_items(std::vector<DisplayItem> items) { this->items = items; }

	void draw(int selected_item_id);
};
