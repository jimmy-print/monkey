#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

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

	Model::Model(std::string objpath)
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

			shader = load_shader("shaders/3d_obj_vs", "shaders/3d_obj_fs");
			lightpos_l = glGetUniformLocation(shader, "lightpos");
			viewpos_l = glGetUniformLocation(shader, "viewpos");
			lightcolor_l = glGetUniformLocation(shader, "lightcolor");
			objectcolor_l = glGetUniformLocation(shader, "objectcolor");
			ambient_strength_l = glGetUniformLocation(shader, "ambient_strength");
			model_l = glGetUniformLocation(shader, "model");
			view_l = glGetUniformLocation(shader, "view");
			proj_l = glGetUniformLocation(shader, "proj");

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

			// Load data into buffers

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			glGenBuffers(1, &VBO_vertices);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
			glEnableVertexAttribArray(0);

			glGenBuffers(1, &VBO_normals);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), &normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
			glEnableVertexAttribArray(1);

			for (int i = 0; i < vertices.size(); i++) {
				if ((i - 1) % 3 == 0) {
					vertices[i] += 1.0;
					//pfa(vertices);
				}
			}

			centered_vertices = vertices;
			original_vertices = vertices;
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

	void Model::rotate(float xdeg, float ydeg, float zdeg) {
		std::vector<glm::vec3> original_vertices_vec3;
		glm::vec3 tmp;
		for (int i = 0; i < original_vertices.size(); i++) {
			if (i % 3 == 0) {
				tmp[0] = original_vertices[i];
			}
			if ((i - 1) % 3 == 0) {
				tmp[1] = original_vertices[i];
			}
			if ((i - 2) % 3 == 0) {
				tmp[2] = original_vertices[i];
				original_vertices_vec3.push_back(tmp);
			}
		}

		assert(original_vertices_vec3.size() == centered_vertices.size() / 3);
		assert(original_vertices_vec3.size() == original_vertices.size() / 3);
		assert(original_vertices_vec3.size() == vertices.size() / 3);

		// Perform rotation on original vertices vec3
		for (int i = 0; i < original_vertices_vec3.size(); i++) {
			original_vertices_vec3[i] = rotate_x_axis(original_vertices_vec3[i], xdeg);
			original_vertices_vec3[i] = rotate_y_axis(original_vertices_vec3[i], ydeg);
			original_vertices_vec3[i] = rotate_z_axis(original_vertices_vec3[i], zdeg);
		}

		// Put new values into float array of centered+rotated vertices
		int ii = 0;
		for (auto vertex_vec3 : original_vertices_vec3) {
			assert(ii % 3 == 0);
			for (int i = 0; i < 3; i++) {
				centered_vertices[ii] = vertex_vec3[i];
				ii++;
			}
		}

		// Transform centered+rotated vertices to actual rendered vertices
		float dx, dy, dz;
		dx = vertices[0] - centered_vertices[0];
		dy = vertices[1] - centered_vertices[1];
		dz = vertices[2] - centered_vertices[2];

		assert(vertices.size() == centered_vertices.size());
		for (int i = 0; i < vertices.size(); i++) {
			if ((i - 0) % 3 == 0) {
				vertices[i] = centered_vertices[i] + dx;
			} else if ((i - 1) % 3 == 0) {
				vertices[i] = centered_vertices[i] + dy;
			} else if ((i - 2) % 3 == 0) {
				vertices[i] = centered_vertices[i] + dz;
			}
		}
	}

	void Model::rotate_to_player(glm::vec3 pos) {
		float dx, dy, dz;
		dx = vertices[0] - centered_vertices[0];
		dy = vertices[1] - centered_vertices[1];
		dz = vertices[2] - centered_vertices[2];

		glm::vec3 centered_pos(pos[0] - dx, pos[1], pos[2] - dz);

		glm::vec2 flat_pos(centered_pos[0], centered_pos[2]);
		glm::vec2 n_flat_pos = glm::normalize(flat_pos);

		float x = n_flat_pos[0];
		float z = n_flat_pos[1];

		float rot;
		if (centered_pos[2] < 0) {
			rot = 180 + deg(asin(x));
		} else {
			rot = deg(asin(-x));
		}

		if (!isnan(rot)) {
			// if pos is at center of obj
			// presumably caused by error with trigonometric functions
			rotate(0, rot, 0);
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
