#ifndef OBJ_H
#define OBJ_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

std::vector<std::string> split(std::string s);

struct shared {
	std::vector<float> centered_vertices;
	std::vector<std::vector<float >> centered_rotated_vertices;
	// vector of float vectors, with each float vector being
	// offset from centered_vertices by 1 additional deg. increment (or rad)
	std::vector<float> normals;
	std::vector<float> uvs;

	// NOTE: i am not sure if uvs are normalised like normals.
};

struct shared get_shared_data(std::string objpath);

class Model {
public:
	std::vector<float> vertices;  
	// actual, rendered vertices which are unique to each monkey

	struct shared *s;

	float x = 0;
	float z = 0;

	float displacement_x = 0;
	float displacement_y = 0;
   	float displacement_z = 0;

	// uniform locations
	GLuint lightpos_l;
	GLuint viewpos_l;
	GLuint lightcolor_l;
	GLuint objectcolor_l;
	GLuint ambient_strength_l;

	GLuint model_l;
	GLuint view_l;
	GLuint proj_l;

	GLuint shader;

	GLuint VAO;
	GLuint VBO_vertices, VBO_normals;
	Model(struct shared *sp);
	~Model();
	void move_x(float dx);
	void move_y(float dy);
	void move_z(float dz);
	void move_to_player(glm::vec3 pos);
	void rotate(float xdeg, float ydeg, float zdeg);
	void rotate_to_player(glm::vec3 pos);
	void draw(glm::vec3 pos, glm::vec3 lightpos, glm::mat4 model, glm::mat4 view, glm::mat4 proj, glm::vec3 color);
	bool hits(glm::vec3 pos, glm::vec3 direction);
};

#endif
