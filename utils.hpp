#ifndef UTILS_H
#define UTILS_H

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define PI 3.141592653589793

void pfa(std::vector<float> v)
{
    for (std::vector<float>::const_iterator i = v.begin(); i != v.end(); i++)
        std::cout << *i << " ";
    std::cout << "\n";
}

float rad(float deg) {
	return deg * PI / 180;
}

glm::vec3 rotate_x_axis(glm::vec3 orig, float degree) {
	float rotation_matrix[9] = {
		1, 0, 0,
		0, cos(rad(degree)), -sin(rad(degree)),
		0, sin(rad(degree)), cos(rad(degree))};
	glm::mat3 m;
	memcpy(glm::value_ptr(m), rotation_matrix, sizeof(rotation_matrix));
	return m * orig;
}

glm::vec3 rotate_y_axis(glm::vec3 orig, float degree) {
    float rotation_matrix[9] = {
        cos(rad(degree)), 0, sin(rad(degree)),
        0, 1, 0,
        -sin(rad(degree)), 0, cos(rad(degree))};
	glm::mat3 m;
	memcpy(glm::value_ptr(m), rotation_matrix, sizeof(rotation_matrix));
	return m * orig;
}

void print_vec3(glm::vec3 v)
{
	std::cout << v[0] << " " << v[1] << " " << v[2] << "\n";
}

#endif
