#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utils.h>

// these below may be redundant
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

void pfa(std::vector<float> v)
{
	for (std::vector<float>::const_iterator i = v.begin(); i != v.end(); i++)
		std::cout << *i << " ";
	std::cout << "\n";
}

float rad(float deg) {
	return deg * PI / 180;
}

float deg(float rad) {
	return 180 * rad / PI;
}

glm::vec3 rotate_x_axis(glm::vec3 orig, float degree) {
	float rotation_matrix[9] = {
		1, 0, 0,
		0, (float) cos(rad(degree)), (float) -sin(rad(degree)),
		0, (float) sin(rad(degree)), (float) cos(rad(degree))};
	glm::mat3 m;
	memcpy(glm::value_ptr(m), rotation_matrix, sizeof(rotation_matrix));
	return m * orig;
}

glm::vec3 rotate_y_axis(glm::vec3 orig, float degree) {
	float rotation_matrix[9] = {
		(float) cos(rad(degree)), 0, (float) sin(rad(degree)),
		0, 1, 0,
		(float) -sin(rad(degree)), 0, (float) cos(rad(degree))};
	glm::mat3 m;
	memcpy(glm::value_ptr(m), rotation_matrix, sizeof(rotation_matrix));
	return m * orig;
}

glm::vec3 rotate_z_axis(glm::vec3 orig, float degree) {
	float rotation_matrix[9] = {
		(float) cos(rad(degree)), (float) -sin(rad(degree)), 0,
		(float) sin(rad(degree)), (float) cos(rad(degree)), 0,
		0, 0, 1};
	glm::mat3 m;
	memcpy(glm::value_ptr(m), rotation_matrix, sizeof(rotation_matrix));
	return m * orig;
}

void print_vec3(glm::vec3 v)
{
	std::cout << v[0] << " " << v[1] << " " << v[2] << "\n";
}

std::string get_file_str(std::string file)
{
	std::ifstream f(file);
	if (f.fail()) {
		std::cout << file << " opening failed\n";
	}
	std::string s((std::istreambuf_iterator<char>(f)),
		      (std::istreambuf_iterator<char>()));
	return s;
}

GLuint load_shader(std::string vs_filename, std::string fs_filename)
{
	const char *vs_cstr;
	const char *fs_cstr;
	std::string vs_str = get_file_str(vs_filename);
	std::string fs_str = get_file_str(fs_filename);
	vs_cstr = vs_str.c_str();
	fs_cstr = fs_str.c_str();

	GLuint program;

	program = glCreateProgram();

	GLuint vs, fs;
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_cstr, NULL);
	glCompileShader(vs);

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_cstr, NULL);
	glCompileShader(fs);

	// Check for errors
	GLint vs_success, fs_success;
	vs_success = 0;
	fs_success = 0;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &vs_success);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &fs_success);

	if (vs_success == GL_FALSE) {
		GLint vs_max_len;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &vs_max_len);
		std::vector<GLchar> vs_error_log(vs_max_len);
		glGetShaderInfoLog(vs, vs_max_len, &vs_max_len, &vs_error_log[0]);

		std::cout << "ERROR: Vertex shader compilation\n";
		for (auto c : vs_error_log) {
			std::cout << c;
		}
		std::cout << "\n";
	}
	if (fs_success == GL_FALSE) {
		GLint fs_max_len;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &fs_max_len);
		std::vector<GLchar> fs_error_log(fs_max_len);
		glGetShaderInfoLog(fs, fs_max_len, &fs_max_len, &fs_error_log[0]);

		std::cout << "ERROR: Fragment shader compilation\n";
		for (auto c : fs_error_log) {
			std::cout << c;
		}
	}

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}


