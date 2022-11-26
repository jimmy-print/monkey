#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <obj.h>
#include <utils.h>
#include <math.h>

std::vector<std::string> split(std::string s) {
	std::vector<std::string> v;
	std::string tmp;
	for (auto c : s) {
		if (c != ' ') {
			tmp += c;
		} else {
			v.push_back(tmp);
			tmp = "";
		}
	}
	v.push_back(tmp); // I think this assumes no trailing whitespace
	return v;
}

struct shared get_shared_data(std::string objpath)
{
	std::string filestr = get_file_str(objpath);
	std::istringstream stream(filestr);
	std::string line;

	std::vector<GLuint> vertex_indices;
	std::vector<GLuint> uv_indices;
	std::vector<GLuint> normal_indices;

	std::vector<glm::vec3> tmp_vertices;
	std::vector<glm::vec2> tmp_uvs;
	std::vector<glm::vec3> tmp_normals;

	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<float> normals;

	while (std::getline(stream, line)) {
		std::vector<std::string> vector;
		vector = split(line);
		if (vector[0] == "v") {
			assert(vector.size() == 4);
			glm::vec3 vertex(stof(vector[1]), stof(vector[2]), stof(vector[3]));
			tmp_vertices.push_back(vertex);
		}
		else if (vector[0] == "vt") {
			assert(vector.size() == 3);
			glm::vec2 uv(stof(vector[1]), stof(vector[2]));
			tmp_uvs.push_back(uv);
		}
		else if (vector[0] == "vn") {
			assert(vector.size() == 4);
			glm::vec3 normal(stof(vector[1]), stof(vector[2]), stof(vector[3]));
			tmp_normals.push_back(normal);
		}
		else if (vector[0] == "f") {
			assert(vector.size() == 4);
			std::string v1, v2, v3;
			GLuint vertex_index[3], uv_index[3], normal_index[3];
			sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertex_index[0], &uv_index[0], &normal_index[0], &vertex_index[1], &uv_index[1], &normal_index[1], &vertex_index[2], &uv_index[2], &normal_index[2]);
			vertex_indices.push_back(vertex_index[0]);
			vertex_indices.push_back(vertex_index[1]);
			vertex_indices.push_back(vertex_index[2]);
			uv_indices.push_back(uv_index[0]);
			uv_indices.push_back(uv_index[1]);
			uv_indices.push_back(uv_index[2]);
			normal_indices.push_back(normal_index[0]);
			normal_indices.push_back(normal_index[1]);
			normal_indices.push_back(normal_index[2]);
		}
	}
	std::vector<glm::vec3> inter_vertices;
	std::vector<glm::vec2> inter_uvs;
	std::vector<glm::vec3> inter_normals;

	for (int i = 0; i < vertex_indices.size(); i++) {
		unsigned int vertex_index = vertex_indices[i];
		unsigned int uv_index = uv_indices[i];
		unsigned int normal_index = normal_indices[i];
		glm::vec3 vertex = tmp_vertices[vertex_index - 1];
		//glm::vec2 uv = tmp_uvs[uv_index - 1];
		glm::vec3 normal = tmp_normals[normal_index - 1];

		inter_vertices.push_back(vertex);
		inter_normals.push_back(normal);
	}

	// change glm::vec3 vectors to float vectors
	for (auto vec : inter_vertices)
	{
		vertices.push_back(vec.x);
		vertices.push_back(vec.y);
		vertices.push_back(vec.z);
	}

	for (auto vec : inter_normals)
	{
		normals.push_back(vec.x);
		normals.push_back(vec.y);
		normals.push_back(vec.z);
	}

	for (int i = 0; i < vertices.size(); i++) {
		if ((i - 1) % 3 == 0) {
			vertices[i] += 1.0;  // so that the models are above ground
		}
	}


	// generate rotated copies of vertices
	std::vector<std::vector<float>> centered_rotated_vertices;
	int d = 0;  // the current degree of the rotated vertices
	// is also the index of the rotated vertices in the 2D vector.

	assert(vertices.size() % 3 == 0);

	for (d = 0; d > -360; d--) {
		std::vector<glm::vec3> tmp;
		glm::vec3 tmpp;
		for (int i = 0; i < vertices.size(); i++) {
			if ((i - 0) % 3 == 0) {
				tmpp[0] = vertices[i];
			}
			else if ((i - 1) % 3 == 0) {
				tmpp[1] = vertices[i];
			}
			else if ((i - 2) % 3 == 0) {
				tmpp[2] = vertices[i];
				tmp.push_back(rotate_y_axis(tmpp, d));
			}
		}
		assert(tmp.size() == vertices.size() / 3);
		std::vector<float> tmp_in_float;
		for (auto v : tmp) {
			tmp_in_float.push_back(v[0]);
			tmp_in_float.push_back(v[1]);
			tmp_in_float.push_back(v[2]);
		}
		assert(tmp_in_float.size() == tmp.size() * 3);

		centered_rotated_vertices.push_back(tmp_in_float);
	}

	struct shared s = {
		vertices,
		centered_rotated_vertices,
		normals,
		uvs
	};
	return s;
}

Model::Model(struct shared *sp)
{
	shader = load_shader("shaders/3d_obj_vs", "shaders/3d_obj_fs");
	lightpos_l = glGetUniformLocation(shader, "lightpos");
	viewpos_l = glGetUniformLocation(shader, "viewpos");
	lightcolor_l = glGetUniformLocation(shader, "lightcolor");
	objectcolor_l = glGetUniformLocation(shader, "objectcolor");
	ambient_strength_l = glGetUniformLocation(shader, "ambient_strength");
	model_l = glGetUniformLocation(shader, "model");
	view_l = glGetUniformLocation(shader, "view");
	proj_l = glGetUniformLocation(shader, "proj");

	s = sp;
	vertices = s->centered_vertices;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &VBO_normals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * s->normals.size(), &s->normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
}

Model::~Model()
{
	// Same issue as in platformer
	// The deletion stops memory usage from growing endlessly,
	// but it doesn't make it decrease.

	// 2021-08-07 NOTE: This doesn't happen on Windows.
	// On Windows, after a monkey is destroyed, the memory usually
	// decreases slightly. It's probably OS-dependent.
	// However, the memory still creeps up slowly even when there's 0-1
	// monkeys that are constantly being destroyed and re-created.
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO_vertices);
	glDeleteBuffers(1, &VBO_normals);  // Could all monkeys share the same VBO_normals?
}

void Model::move_x(float dx) {
	for (int i = 0; i < vertices.size(); i++) {
		if (i %3 == 0){
			vertices[i] += dx;
		}
	}
}

void Model::move_y(float dy) {
	for (int i = 0; i < vertices.size(); i++) {
		if ((i - 1) % 3 == 0) {
			vertices[i] += dy;
		}
	}
}

void Model::move_z(float dz) {
	for (int i = 0; i < vertices.size(); i++) {
		if ((i - 2) % 3 == 0) {
			vertices[i] += dz;
		}
	}
}

void Model::move_to_player(glm::vec3 pos) {
	float rise = pos[0] - vertices[0];
	float run = pos[2] - vertices[2];

	float dx, dy;
	if (run != 0) {
		float m = rise / run;
		float length = sqrt(pow(rise, 2) + pow(run, 2));
		float length_per_x1 = length / run;

		dx = 1 / length_per_x1;
		dy = 1 / length_per_x1 * m;
	} else {
		dx = 0;
		dy = (rise > 0) ? 1 : -1;
	}

	dx *= 0.03;
	dy *= 0.03;

	for (int i = 0; i < vertices.size(); i++) {
		if (i % 3 == 0) {
			vertices[i] += dy;
		} else if ((i - 2) % 3 == 0) {
			vertices[i] += dx;
		}
	}
}

void Model::rotate_to_player(glm::vec3 pos) {
	float dx, dy, dz;
	dx = vertices[0] - s->centered_vertices[0];
	dy = vertices[1] - s->centered_vertices[1];
	dz = vertices[2] - s->centered_vertices[2];

	glm::vec3 centered_pos(pos[0] - dx, pos[1], pos[2] - dz);

	glm::vec2 flat_pos(centered_pos[0], centered_pos[2]);
	glm::vec2 n_flat_pos = glm::normalize(flat_pos);

	float x = n_flat_pos[0];
	float z = n_flat_pos[1];

	float rot;

	int quadrant;
	if (centered_pos.z > 0 && centered_pos.x > 0)
		quadrant = 1;
	else if (centered_pos.z < 0 && centered_pos.x > 0)
		quadrant = 2;
	else if (centered_pos.z < 0 && centered_pos.x < 0)
		quadrant = 3;
	else if (centered_pos.z > 0 && centered_pos.x < 0)
		quadrant = 4;

	switch (quadrant) {
	case 2:
	case 3:
		rot = 180 - deg(asin(x));
		break;
	case 1:
		rot = deg(asin(-x));
		break;
	case 4:
		rot = -deg(asin(-x));
		break;
	}

	// TODO: simplify logic

	if (!isnan(rot)) {
		// if pos is at center of obj
		// presumably caused by error with trigonometric functions
		// rot = abs(rot);
		int rot_i;
		rot_i = abs(round(rot));

		if (quadrant == 4) {
			rot_i = -rot_i;
			rot_i += 360;
			if (rot_i >= 360) {
				rot_i = 359;
			}
		}

		float saved_x = vertices[0];
		float saved_y = vertices[1];
		float saved_z = vertices[2];

		vertices = s->centered_rotated_vertices[rot_i];

		float dx = saved_x - s->centered_rotated_vertices[rot_i][0];
		float dy = saved_y - s->centered_rotated_vertices[rot_i][1];
		float dz = saved_z - s->centered_rotated_vertices[rot_i][2];

		for (int i = 0; i < vertices.size(); i++) {
			if ((i - 0) % 3 == 0) {
				vertices[i] += dx;
			}
			else if ((i - 1) % 3 == 0) {
				vertices[i] += dy;
			}
			else if ((i - 2) % 3 == 0) {
				vertices[i] += dz;
			}
		}
	}
}

void Model::draw(glm::vec3 pos, glm::vec3 lightpos, glm::mat4 model, glm::mat4 view, glm::mat4 proj)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glUseProgram(shader);

	glUniform3f(lightcolor_l, 1.0, 1.0, 1.0);
	glUniform3f(objectcolor_l, 1.0, 1.0, 1.0);
	glUniform3f(viewpos_l, pos[0], pos[1], pos[2]);
	glUniform3f(lightpos_l, lightpos[0], lightpos[1], lightpos[2]);
	glUniform1f(ambient_strength_l, 1.f);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glUniformMatrix4fv(model_l, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(view_l, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(proj_l, 1, GL_FALSE, &proj[0][0]);
}

bool intersect_ray_triangle(glm::vec3 pos, glm::vec3 direction, glm::vec3 tri_p1, glm::vec3 tri_p2, glm::vec3 tri_p3)
{
	const float EPSILON = 0.0000001;
	glm::vec3 edge1, edge2, h, s, q;
	float a, f, u, v;

	edge1 = tri_p2 - tri_p1;
	edge2 = tri_p3 - tri_p1;

	h = glm::cross(direction, edge2);
	a = glm::dot(edge1, h);
	if (a > -EPSILON && a < EPSILON) {
		return false;
	}

	f = 1.0 / a;
	s = pos - tri_p1;
	u = f * glm::dot(s, h);
	if (u < 0.0 || u > 1.0) {
		return false;

	}

	q = glm::cross(s, edge1);
	v = f * glm::dot(direction, q);
	if (v < 0.0 || u + v > 1.0) {
		return false;
	}

	float t = f * glm::dot(edge2, q);
	if (t > EPSILON) {
		return true;
	} else {
		return false;
	}
}

bool Model::hits(glm::vec3 pos, glm::vec3 direction) {
	// This function does not return true when the pos is above the model and
	// the direction is facing downwards. (when f is pressed, for freecam)
	std::vector<glm::vec3> vertices_as_vec3;
	std::vector<float> tmp_vertex;

	for (int i = 1; i <= vertices.size(); i++) {
		tmp_vertex.push_back(vertices[i - 1]);
		if (i % 3 == 0) {
			vertices_as_vec3.push_back(glm::vec3(tmp_vertex[0], tmp_vertex[1], tmp_vertex[2]));
			tmp_vertex.clear();
			if (i % 9 == 0) {
				if (intersect_ray_triangle(pos, direction, vertices_as_vec3[0], vertices_as_vec3[1], vertices_as_vec3[2])) {
					return true;
				}
				vertices_as_vec3.clear();
			}
		}
	}

	return false;
}
