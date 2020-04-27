#pragma once

#include <vector>
#include <functional>
#include "ImageManager.hpp"
#include "TextManager.hpp"
#include "Image.hpp"
#include "DisplayItem.hpp"
#include "Bounds.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class Display {
	const ImageManager& image_manager;
	const TextManager& text_manager;
	int texture_revision = -1;
	std::vector<DisplayItem> items;
	std::vector<float> floats;
	std::vector<unsigned int> indices;
	std::vector<float> text_floats;
	std::vector<unsigned int> text_indices;
	int width;
	int height;
	GLuint vao;
	GLuint vbo[2];
	GLuint ebo[2];
	GLuint texture_id[2];
	GLuint shader_program;

	size_t old_num_floats = 0;
	size_t old_num_text_floats = 0;

	void update_geometry(int selected_item);

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

	void add_point(float x, float y, float s, float t) {
		floats.push_back(x);
		floats.push_back(y);
		floats.push_back(s);
		floats.push_back(t);
	}

	void add_text_point(float x, float y, float s, float t) {
		text_floats.push_back(x);
		text_floats.push_back(y);
		text_floats.push_back(s);
		text_floats.push_back(t);
	}

	int add_indices( int index) {
		indices.push_back(index + 0);
		indices.push_back(index + 1);
		indices.push_back(index + 3);
		indices.push_back(index + 1);
		indices.push_back(index + 2);
		indices.push_back(index + 3);
		return index + 4;
	}

	int add_text_indices(int index) {
		text_indices.push_back(index + 0);
		text_indices.push_back(index + 1);
		text_indices.push_back(index + 3);
		text_indices.push_back(index + 1);
		text_indices.push_back(index + 2);
		text_indices.push_back(index + 3);
		return index + 4;
	}

	int add_quad(int index, const Bounds& bounds, const Bounds& tex_bounds) {
		add_point(bounds.right, bounds.top, tex_bounds.right, tex_bounds.top);
		add_point(bounds.right, bounds.bot, tex_bounds.right, tex_bounds.bot);
		add_point(bounds.left, bounds.bot, tex_bounds.left, tex_bounds.bot);
		add_point(bounds.left, bounds.top, tex_bounds.left, tex_bounds.top);
		return add_indices(index);
	}

	int add_text_quad(int index, const Bounds& bounds, const Bounds& tex_bounds) {
		add_text_point(bounds.right, bounds.top, tex_bounds.right, tex_bounds.top);
		add_text_point(bounds.right, bounds.bot, tex_bounds.right, tex_bounds.bot);
		add_text_point(bounds.left, bounds.bot, tex_bounds.left, tex_bounds.bot);
		add_text_point(bounds.left, bounds.top, tex_bounds.left, tex_bounds.top);
		return add_text_indices(index);
	}

public:
	Display(
		const ImageManager& image_manager,
		const TextManager& text_manager,
		int width,
		int height);

	void set_items(std::vector<DisplayItem> items) { this->items = items; }

	void draw(int selected_item_id);
};
