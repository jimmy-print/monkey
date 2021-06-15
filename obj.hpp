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

#include "utils.hpp"

GLuint load_shader(std::string vs_filename, std::string fs_filename);
std::string get_file_str(std::string file);
void print_vec3(glm::vec3 vec3);

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

void read_obj_file(std::string filename, std::vector<glm::vec3>* verticesp, std::vector<glm::vec2>* uvsp, std::vector<glm::vec3>* normalsp)
{
	std::string filestr = get_file_str(filename);
	std::istringstream stream(filestr);
	std::string line;

	std::vector<GLuint> vertex_indices;
	std::vector<GLuint> uv_indices;
	std::vector<GLuint> normal_indices;
	std::vector<glm::vec3> tmp_vertices;
	std::vector<glm::vec2> tmp_uvs;
	std::vector<glm::vec3> tmp_normals;
	
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
			std::cout << "when exporting obj in blender check triangulate faces option\n";
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
	for (int i = 0; i < vertex_indices.size(); i++) {
		unsigned int vertex_index = vertex_indices[i];
		unsigned int uv_index = uv_indices[i];
		unsigned int normal_index = normal_indices[i];
		glm::vec3 vertex = tmp_vertices[vertex_index - 1];
//			glm::vec2 uv = tmp_uvs[uv_index - 1];
		glm::vec3 normal = tmp_normals[normal_index - 1];

		verticesp->push_back(vertex);
//				uvsp->push_back(uv);
		normalsp->push_back(normal);
	}
}

class Model
{
public:
	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<float> normals;

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
	Model(std::string objpath)
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
		}

	void move_x(float dx) {
		for (int i = 0; i < vertices.size(); i++) {
			if (i %3 == 0){
				vertices[i] += dx;
			}
		}
	}

	void move_y(float dy) {
		for (int i = 0; i < vertices.size(); i++) {
			if ((i - 1) % 3 == 0) {
				vertices[i] += dy;
			}
		}
	}

	void move_z(float dz) {
		for (int i = 0; i < vertices.size(); i++) {
			if ((i - 2) % 3 == 0) {
				vertices[i] += dz;
			}
		}
	}

    void rotate() {
        // Split float array of vertices into glm::vec3 array
        std::vector<glm::vec3> vertices_vec3;
        glm::vec3 tmp;
        for (int i = 0; i < vertices.size(); i++) {
            if (i % 3 == 0) {
                tmp[0] = vertices[i];
            }
            if ((i - 1) % 3 == 0) {
                tmp[1] = vertices[i];
            }
            if ((i - 2) % 3 == 0) {
                tmp[2] = vertices[i];
                vertices_vec3.push_back(tmp);
            }
        }

        assert(vertices_vec3.size() == vertices.size() / 3);
        // Perform rotation
        for (int i = 0; i < vertices_vec3.size(); i++) {
            vertices_vec3[i] = rotate_y_axis(vertices_vec3[i], 1);
            vertices_vec3[i] = rotate_x_axis(vertices_vec3[i], 0.5);
        }

        // Put new values into float array of vertices
        int ii = 0;
        for (auto vertex_vec3 : vertices_vec3) {
            assert(ii % 3 == 0);
            for (int i = 0; i < 3; i++) {
                vertices[ii] = vertex_vec3[i];
                ii++;
            }
        }

        std::cout << vertices[0] << "\n";
    }

	void draw(glm::vec3 pos, glm::vec3 lightpos, glm::mat4 model, glm::mat4 view, glm::mat4 proj)
		{
			lightpos[0] = vertices[0];
			lightpos[2] = vertices[2];
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
};

#endif
