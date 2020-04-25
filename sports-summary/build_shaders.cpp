#include <iostream>
#include <gsl/gsl>
#include "build_shaders.hpp"
#include "utils.hpp"
#include "errors.hpp"


using std::string;

const int MESSAGE_MAX_SIZE = 512;

static GLuint load_shader(const char* filename, GLuint shader_type) {
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

static GLuint link_shaders(GLuint vertex_id, GLuint fragment_id) {
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

GLuint build_shaders(const char* vertex_shader_file, const char* fragment_shader_file) {
	auto vertex_shader = load_shader(vertex_shader_file, GL_VERTEX_SHADER);
	auto delete_vertex_shader = gsl::finally([vertex_shader] { glDeleteShader(vertex_shader); });

	auto fragment_shader = load_shader(fragment_shader_file, GL_FRAGMENT_SHADER);
	auto delete_fragment_shader = gsl::finally([fragment_shader] { glDeleteShader(fragment_shader); });

	return link_shaders(vertex_shader, fragment_shader);
}