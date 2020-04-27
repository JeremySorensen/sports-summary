#pragma once
#include <cstdint>
#include <unordered_map>
#include <optional>
#include <gsl/gsl>
#include <fstream>
#include "load_image.hpp"
#include "Image.hpp"
#include "Bounds.hpp"
#include "errors.hpp"

class TextureManager {
	Image texture;
	int start_x;
	int start_y;
	int max_sub_y = 0;
	std::unordered_map<int, Bounds> id_to_bounds;
	int max_images = 0;
	int num_images = 0;

public:
	TextureManager(
		Image base_image,
		size_t max_width,
		size_t max_height,
		int start_x,
		int start_y) : texture(base_image), start_x(start_x), start_y(start_y) {
		texture.bytes.resize(max_width * max_height * 3);
		texture.width = max_width;
		texture.height = max_height;
	}

	void set_max_images(int max_images) { this->max_images = max_images; }

	void add_image(int id, Image new_image) {

		if (num_images == max_images) {
			logic_error("Tried to add too many images", __FILE__, __LINE__);
		}

		++num_images;

		if (start_x + new_image.width > texture.width) {
			start_x = 0;
			start_y += max_sub_y;
			max_sub_y = 0;
		}

		if (new_image.height > max_sub_y) {
			max_sub_y = new_image.height;
		}

		if (start_y + max_sub_y > texture.height) {
			logic_error("New image doesn't fit in texure!", __FILE__, __LINE__);
		}

		id_to_bounds[id] = { 
			static_cast<float>(start_x),
			static_cast<float>(start_x + new_image.width),
			static_cast<float>(start_y),
			static_cast<float>(start_y + new_image.height) };
 
		for (int i = 0; i < new_image.height; ++i) {
			int tex_row = (i + start_y) * texture.width * 3;
			int img_row = i * new_image.width * 3;
			for (int j = 0; j < new_image.width; ++j) {
				int tex_col = tex_row + (start_x + j) * 3;
				int img_col = img_row + j * 3;
				texture.bytes[tex_col + 0] = new_image.bytes[img_col + 0];
				texture.bytes[tex_col + 1] = new_image.bytes[img_col + 1];
				texture.bytes[tex_col + 2] = new_image.bytes[img_col + 2];
			}
		}

		start_x += new_image.width;
	}

	bool has_all_images() const {
		return num_images == max_images;
	}

	Image get_texture() const { return texture; }

	std::optional<Bounds> get_bounds(int id) const { 
		auto itr = id_to_bounds.find(id);
		return itr == id_to_bounds.end() ? std::optional<Bounds>() : std::optional(itr->second);
	}
};