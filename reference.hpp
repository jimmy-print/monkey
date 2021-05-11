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

		shader = load_shader("objvs", "objfs");
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

	void draw(glm::vec3 pos, glm::vec3 lightpos, glm::mat4 model, glm::mat4 view, glm::mat4 proj)
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
        for (int i = 0; i < vertices.size(); i++) {
            if ((i-2)%3 == 0){
        //        vertices[i] += 0.05;
            }
        }
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

/*
int main()
{
	
	const int D_WIDTH = 1920;
	const int D_HEIGHT = 1080;
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow *wind = glfwCreateWindow(D_WIDTH, D_HEIGHT, "foo", 0, 0);
	glfwMakeContextCurrent(wind);
	glewInit();
	glEnable(GL_MULTISAMPLE);  

	float FOV = 45;
	glm::mat4 proj = glm::perspective(glm::radians(FOV), (float) D_WIDTH / (float) D_HEIGHT, 0.1f, 100.0f);
	glm::vec3 pos(0, 1, 0);
	glm::mat4 view = glm::lookAt(pos,
				     glm::vec3(0, 0, -5),
				     glm::vec3(0, 1, 0));
	glm::mat4 model(1.0f);
	float horizontal_angle, vertical_angle;
	float mouse_speed = 0.01;
	float travel_speed = 0.5;
	float delay = 0.1;
	double mouse_x, mouse_y;
	glfwSetInputMode(wind, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Model m("design.obj");
	Model n("teapot.obj");
	Model e("e.obj");

	glm::vec3 lightpos(5, 5, 0);

	glm::mat4 mvp;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	while (!glfwWindowShouldClose(wind)) {
		glClearColor(0.0, 0.5, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();

//		lightpos = pos;

		glfwGetCursorPos(wind, &mouse_x, &mouse_y);
		horizontal_angle = mouse_speed * delay * float(D_WIDTH / 2 - mouse_x);
		vertical_angle = mouse_speed * delay * float(D_HEIGHT / 2 - mouse_y);
		glm::vec3 direction(cos(vertical_angle) * sin(horizontal_angle),
				    sin(vertical_angle),
				    cos(vertical_angle) * cos(horizontal_angle));
		glm::vec3 right(sin(horizontal_angle - 3.14f / 2.0f),
				0,
				cos(horizontal_angle - 3.14f / 2.0f));
		if (glfwGetKey(wind, GLFW_KEY_W) == GLFW_PRESS) {
			pos += direction * delay * travel_speed;
		}
		if (glfwGetKey(wind, GLFW_KEY_S) == GLFW_PRESS) {
			pos -= direction * delay * travel_speed;
		}
		if (glfwGetKey(wind, GLFW_KEY_D) == GLFW_PRESS) {
			pos += right * delay * travel_speed;
		}
		if (glfwGetKey(wind, GLFW_KEY_A) == GLFW_PRESS) {
			pos -= right * delay * travel_speed;
		}

		glm::vec3 up = glm::cross(right, direction);
		// I think this is a different vector with glm::vec3(0, 1, 0)
		// which is a parameter in the view matrix constructor
		view = glm::lookAt(pos, pos + direction, up);
		
		
		mvp = proj * view * model;

		m.draw(pos, lightpos, model, view, proj);
		n.draw(pos, lightpos, model, view, proj);
		e.draw(pos, lightpos, model, view, proj);
		
		glfwSwapBuffers(wind);
	}
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

void print_vec3(glm::vec3 vec3) {
	std::cout << vec3[0] << " " << vec3[1] << " " << vec3[2] << "\n";
}
*/
