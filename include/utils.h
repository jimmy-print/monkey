#ifndef UTILS_H
#define UTILS_H

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>

#define PI 3.141592653589793

#define D_WIDTH 1700
#define D_HEIGHT 900


#define CYAN 0.0, 1.0, 1.0
#define RED  1.0, 0.0, 0.0
#define BLUE 0.0, 0.0, 1.0
#define BLACK 0.0, 0.0, 0.0
#define WHITE 1.0, 1.0, 1.0
#define YELLOW 1.0, 1.0, 0.0
#define GREEN 0.0, 1.0, 0.0

void pfa(std::vector<float> v);

float rad(float deg);
float deg(float rad);

glm::vec3 rotate_x_axis(glm::vec3 orig, float degree);
glm::vec3 rotate_y_axis(glm::vec3 orig, float degree);
glm::vec3 rotate_z_axis(glm::vec3 orig, float degree);

void print_vec3(glm::vec3 v);

std::string get_file_str(std::string file);

GLuint load_shader(std::string vs_filename, std::string fs_filename);

float get_distance(float x0, float z0, float x1, float z1);

#endif
