// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// This class manages pretty much all the OpenGl setup and drawing.

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Image.hpp"
#include "ImageManager.hpp"

class OpenGlDrawer {

	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	GLuint texture_id[2];

	size_t old_num_floats = 0;
	size_t old_num_text_floats = 0;
	int texture_revision = -1;

	GLuint load_shader(const char* filename, GLuint shader_type);
	GLuint link_shaders(GLuint vertex_id, GLuint fragment_id);
	GLuint build_shaders(const char* vertex_shader_file, const char* fragment_shader_file);

public:
	OpenGlDrawer(int width, int height, AlphaImage text_image);

	void draw(
		const std::vector<float>& floats,
		const std::vector<unsigned int>& indices,
		const ImageManager& image_manager);
};
