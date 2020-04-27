#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Image.hpp"
#include "ImageManager.hpp"

class OpenGlDrawer {

	GLuint shader_program;
	GLuint vao;
	GLuint vbo[2];
	GLuint ebo[2];
	GLuint texture_id[2];

	int old_num_floats = 0;
	int old_num_text_floats = 0;
	int texture_revision = -1;

	GLuint load_shader(const char* filename, GLuint shader_type);
	GLuint link_shaders(GLuint vertex_id, GLuint fragment_id);
	GLuint build_shaders(const char* vertex_shader_file, const char* fragment_shader_file);

public:
	OpenGlDrawer(int width, int height, AlphaImage text_image);

	void draw(
		const std::vector<float>& floats,
		const std::vector<float>& text_floats,
		const std::vector<unsigned int>& indices,
		const std::vector<unsigned int>& text_indices,
		const ImageManager& image_manager);

	

};
