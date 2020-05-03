// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// See header file for more information.

#include <iostream>
#include <gsl/gsl>
#include "OpenGlDrawer.hpp"
#include "utils.hpp"
#include "errors.hpp"


using std::string;

const int MESSAGE_MAX_SIZE = 512;

OpenGlDrawer::OpenGlDrawer(int width, int height, AlphaImage text_image) {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		ui_error("Error loading OpenGL", __FILE__, __LINE__);
	}

	shader_program = build_shaders("shaders/vertex.glsl", "shaders/fragment.glsl");

	glViewport(0, 0, width, height);

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenTextures(2, texture_id);

	glBindVertexArray(vao);

	// Text texure setup

	glBindTexture(GL_TEXTURE_2D, texture_id[0]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA8,
		text_image.width,
		text_image.height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		text_image.bytes.data()
	);

	// Geometry texture setup

	glBindTexture(GL_TEXTURE_2D, texture_id[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// mutli-textue setup
	glUseProgram(shader_program);
	glUniform1i(glGetUniformLocation(shader_program, "texture_0"), 0);
	glUniform1i(glGetUniformLocation(shader_program, "texture_1"), 1);
}

void OpenGlDrawer::draw(
	const std::vector<float>& floats,
	const std::vector<unsigned int>& indices,
	const ImageManager& image_manager) {


	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	bool rebuffer = false;
	if (floats.size() > old_num_floats) {
		rebuffer = true;
	}

	if (rebuffer) {
		old_num_floats = floats.size();
		glBufferData(GL_ARRAY_BUFFER, floats.size() * sizeof(float), floats.data(), GL_DYNAMIC_DRAW);
	}
	else {
		glBufferSubData(GL_ARRAY_BUFFER, 0, floats.size() * sizeof(float), floats.data());
	}

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	if (rebuffer) {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
	}
	else {
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(unsigned int), indices.data());
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_id[1]);

	if (image_manager.get_revision() != texture_revision) {
		texture_revision = image_manager.get_revision();

		auto texture = image_manager.get_texture();

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.bytes.data());
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id[0]);

	glUseProgram(shader_program);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
}

GLuint OpenGlDrawer::build_shaders(const char* vertex_shader_file, const char* fragment_shader_file) {
	auto vertex_shader = load_shader(vertex_shader_file, GL_VERTEX_SHADER);
	auto delete_vertex_shader = gsl::finally([vertex_shader] { glDeleteShader(vertex_shader); });

	auto fragment_shader = load_shader(fragment_shader_file, GL_FRAGMENT_SHADER);
	auto delete_fragment_shader = gsl::finally([fragment_shader] { glDeleteShader(fragment_shader); });

	return link_shaders(vertex_shader, fragment_shader);
}

GLuint OpenGlDrawer::load_shader(const char* filename, GLuint shader_type) {
	auto source = read_all_text(filename);
	auto source_str = source.c_str();

	auto shader_id = glCreateShader(shader_type);
	glShaderSource(shader_id, 1, &source_str, nullptr);
	glCompileShader(shader_id);

	int  success;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

	if (!success) {
		string message(MESSAGE_MAX_SIZE, ' ');
		GLsizei length;
		glGetShaderInfoLog(shader_id, gsl::narrow_cast<GLsizei>(message.size()), &length, message.data());
		message.resize(length);
		logic_error(("Error compiling shader: '" + message + "'").c_str(), __FILE__, __LINE__);
	}

	return shader_id;
}

GLuint OpenGlDrawer::link_shaders(GLuint vertex_id, GLuint fragment_id) {
	auto program_id = glCreateProgram();
	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);
	glLinkProgram(program_id);

	int success;
	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	if (!success) {
		string message(MESSAGE_MAX_SIZE, ' ');
		GLsizei length;
		glGetProgramInfoLog(program_id, gsl::narrow_cast<GLsizei>(message.size()), &length, message.data());
		message.resize(length);
		logic_error(("Error linking shader program: '" + message + "'").c_str(), __FILE__, __LINE__);
	}

	return program_id;
}