#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <obj.h>
#include <utils.h>
#include <text.h>

#define ORIGIN glVertex3f(0.0, 0.0, 0.0);

bool click = false;
bool freecam = false;
glm::vec2 result;

// https://stackoverflow.com/a/26221725/11772905
// CC0 1.0
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
	int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
	if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
	auto size = static_cast<size_t>( size_s );
	auto buf = std::make_unique<char[]>( size );
	std::snprintf( buf.get(), size, format.c_str(), args ... );
	return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

int init_time, now_time;
void log(std::string msg)
{
	now_time = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
	float diff = (float) (now_time - init_time) / 1000;
	std::cout << diff << " " << msg << "\n";
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && click == false) {
		click = true;
	} else {
		click = false;
	}
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		freecam = !freecam;
	}
}

float f(float x);

int main()
{
	init_time = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow *window = glfwCreateWindow(D_WIDTH, D_HEIGHT, "monkey", 0, 0);
	glfwMakeContextCurrent(window);
	glewInit();
	glEnable(GL_MULTISAMPLE);

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);

	float FOV = 70;
	glm::mat4 proj = glm::perspective(
		glm::radians(FOV), (float) D_WIDTH / (float) D_HEIGHT, 0.1f, 100.0f);
	float initial_y = 1;
	glm::vec3 pos(5, 0.5, 5);
	glm::mat4 view = glm::lookAt(pos,
				     glm::vec3(0, 0, -5),
				     glm::vec3(0, 1, 0));
	glm::mat4 model(1.0f);
	float horizontal_angle, vertical_angle;
	float horizontal_angle_deg, vertical_angle_deg;
	float mouse_speed = 0.01;
	float travel_speed = 0.4;
	float delay = 0.1;
	double mouse_x, mouse_y;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glm::mat4 mvp;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLuint program_3d_polygon = load_shader("shaders/3d_polygon_vs", "shaders/3d_polygon_fs");
	GLuint program_3d_polygon_mvp_l = glGetUniformLocation(program_3d_polygon, "mvp");

	float vs[] = {
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		1.0, 1.0, 0.0,
		1.0, 0.0, 0.0};
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vs), vs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);

	// image section
	GLuint program_3d_image = load_shader("shaders/3d_image_vs", "shaders/3d_image_fs");
	GLuint program_3d_image_mvp_l = glGetUniformLocation(program_3d_image, "mvp");
	float image_vs[] = {
		-1000.0, 0.0, -1000.0,    0.0, 0.0,
		1000.0, 0.0, -1000.0,   0.0, 100.0,
		1000.0, 0.0, 1000.0,  100.0, 100.0,
		-1000.0, 0.0, 1000.0,   100.0, 0.0
	};
	GLuint image_VAO, image_VBO;
	glGenVertexArrays(1, &image_VAO);
	glBindVertexArray(image_VAO);

	glGenBuffers(1, &image_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, image_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(image_vs), image_vs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (0 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	stbi_set_flip_vertically_on_load(true);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int w, h, n;
	unsigned char* data = stbi_load("Grass.jpg", &w, &h, &n, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	} else {
		std::cout << "Failed to load Grass.jpg" << "\n";
	}
	stbi_image_free(data);

	GLuint program_2d_polygon = load_shader("shaders/2d_polygon_vs", "shaders/2d_polygon_fs");
	GLuint program_2d_polygon_color_l = glGetUniformLocation(program_2d_polygon, "color_l");

	GLuint program_2d_image = load_shader("shaders/2d_image_vs", "shaders/2d_image_fs");
	float gun_vs[] = {
		0.2, -1.0,  0.0, 0.0,
		0.2, -0.2,  0.0, 1.0,
		0.8, -0.2,  1.0, 1.0,
		0.8, -1.0,  1.0, 0.0
	};
	GLuint gun_VAO, gun_VBO;
	glGenVertexArrays(1, &gun_VAO);
	glBindVertexArray(gun_VAO);

	glGenBuffers(1, &gun_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, gun_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gun_vs), gun_vs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (0 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLuint gun_texture;
	glGenTextures(1, &gun_texture);
	glBindTexture(GL_TEXTURE_2D, gun_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int pw, ph, pn;
	unsigned char* pdata = stbi_load("Pistol.png", &pw, &ph, &pn, 0);
	if (pdata) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pw, ph, 0, GL_RGBA, GL_UNSIGNED_BYTE, pdata);
	} else {
		std::cout << "Failed to load Pistol.png" << "\n";
	}
	stbi_image_free(pdata);

	float gun_2_vs[] = {
		0.2, -1.0,  0.0, 0.0,
		0.2, -0.2,  0.0, 1.0,
		0.8, -0.2,  1.0, 1.0,
		0.8, -1.0,  1.0, 0.0
	};
	GLuint gun_2_VAO, gun_2_VBO;
	glGenVertexArrays(1, &gun_2_VAO);
	glBindVertexArray(gun_2_VAO);

	glGenBuffers(1, &gun_2_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, gun_2_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gun_2_vs), gun_2_vs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (0 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	GLuint gun_2_texture;
	glGenTextures(1, &gun_2_texture);
	glBindTexture(GL_TEXTURE_2D, gun_2_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int ppw, pph, ppn;
	unsigned char* ppdata = stbi_load("Pistol-2.png", &ppw, &pph, &ppn, 0);
	if (ppdata) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ppw, pph, 0, GL_RGBA, GL_UNSIGNED_BYTE, ppdata);
	} else {
		std::cout << "Failed to load Pistol-2.png" << "\n";
	}
	stbi_image_free(ppdata);

	Model monkey("obj.obj");
	Model m1("obj.obj");
	Model m2("obj.obj");
	Model m3("obj.obj");

	std::vector<Model> monkeys = {monkey, m1, m2, m3};

	monkeys[1].move_x(10);
	monkeys[2].move_z(10);
	monkeys[3].move_x(10);
	monkeys[3].move_z(10);

	glm::vec3 d;  // 3D unit direction vector
	// Converted from spherical coordinate system variables (horizontal_angle, vertical_angle)
	// Different from glm::vec3 direction, which is used to calculate the view matrix.

	glm::vec3 lightpos(0.0, 10.0, 0.0);

	float dy = 0;
	float gravity_const = 0.008;
	float jump_strength = 0.1;
	bool in_air = false;

	float slowdown_multiplier = 0.5;
	float actual_travel_speed = travel_speed;

	float bullet_x = 0.214;
	float point_size = 10;
	bool bullet_moving;
	float crouch = 0.3;

	glm::vec3 point(1, 1, 1);

	std::map<char, glyph> glyphs = init_glyphs("LiberationMono-Regular.ttf");
	configure_blending();
	GLuint text_VAO, text_VBO;
	get_vao_vbo(&text_VAO, &text_VBO);
	glm::mat4 ortho_mvp = get_mvp();
	GLuint program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l;
	get_shader("shaders/2d_text_vs", "shaders/2d_text_fs", &program_2d_text, &program_2d_text_mvp_l, &program_2d_text_color_l);

	while (!glfwWindowShouldClose(window)) {
		if (bullet_moving) {
			point_size -= 2;
			bullet_x -= 0.05;
		}
		if (bullet_x < 0) {
			bullet_x = 0.214;
			point_size = 10;
			bullet_moving = false;
		}
		glClearColor(0.0, 0.5, 0.9, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();

		glfwGetCursorPos(window, &mouse_x, &mouse_y);
		glm::mat4 proj = glm::perspective(
			glm::radians(FOV), (float) D_WIDTH / (float) D_HEIGHT, 0.1f, 100.0f);
		horizontal_angle = mouse_speed * delay * float(D_WIDTH / 2 - mouse_x);
		vertical_angle = mouse_speed * delay * float(D_HEIGHT / 2 - mouse_y);
		horizontal_angle_deg = horizontal_angle * 180 / PI;
		vertical_angle_deg = vertical_angle * 180 / PI;
		if (vertical_angle_deg < -90.0f) {
			mouse_y = (D_HEIGHT / 2) - (rad(-90.0f) / (mouse_speed * delay));
			vertical_angle = rad(-90.0f);
			glfwSetCursorPos(window, mouse_x, mouse_y);
		}
		if (vertical_angle_deg > 90.0f) {
			mouse_y = (D_HEIGHT / 2) - (rad(90.0f) / (mouse_speed * delay));
			vertical_angle = rad(90.0f);
			glfwSetCursorPos(window, mouse_x, mouse_y);
		}

		glm::vec3 direction(cos(vertical_angle) * sin(horizontal_angle),
				    sin(vertical_angle),
				    cos(vertical_angle) * cos(horizontal_angle));

		glm::vec3 right(sin(horizontal_angle - 3.14f / 2.0f),
				0,
				cos(horizontal_angle - 3.14f / 2.0f));
		glm::vec3 direction_without_pitch(sin(horizontal_angle), 0, cos(horizontal_angle));

		if (in_air) {
			actual_travel_speed = travel_speed * slowdown_multiplier;
		} else {
			actual_travel_speed = travel_speed;
		}

		if (freecam) {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				pos += direction * delay * travel_speed;
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				pos -= direction * delay * travel_speed;
			}
		} else {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				pos += direction_without_pitch * delay * actual_travel_speed;
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				pos -= direction_without_pitch * delay * actual_travel_speed;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			pos += right * delay * actual_travel_speed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			pos -= right * delay * actual_travel_speed;
		}

		if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
			FOV = 20;
		} else {
			FOV = 70;
		}


		if (!freecam) {
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
				if (pos[1] == initial_y) {
					dy = jump_strength;
					in_air = true;
				}
			}
			dy -= gravity_const;

			pos[1] += dy;

			if (pos[1] <= initial_y) {
				dy = 0;
				pos[1] = initial_y;
				in_air = false;
			}
			if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !in_air) {
				pos[1] = initial_y - crouch;
			}
		}

		glm::vec3 up = glm::cross(right, direction);
		view = glm::lookAt(pos, pos + direction, up);

		mvp = proj * view * model;

		for (auto m : monkeys) {
			m.rotate_to_player(pos);
			m.draw(pos, lightpos, model, view, proj);
		}

		for (int i = 0; i < monkeys.size(); i++) {
			monkeys[i].move_to_player(pos);
		}

		

		glUseProgram(program_3d_polygon);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glDrawArrays(GL_QUADS, 0, sizeof(vs) / sizeof(vs[0]));
		glUniformMatrix4fv(program_3d_polygon_mvp_l, 1, GL_FALSE, &mvp[0][0]);

		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(image_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, image_VBO);
		glUseProgram(program_3d_image);
		glDrawArrays(GL_QUADS, 0, 12);
		glUniformMatrix4fv(program_3d_image_mvp_l, 1, GL_FALSE, &mvp[0][0]);

		glUseProgram(program_3d_polygon);
//        glUniform3f(program_3d_polygon_color_l, 1.0, 1.0, 1.0);
//		draw_tris(t);

		glUseProgram(program_3d_polygon);
		glBegin(GL_LINES);

		ORIGIN
			glVertex3f(10.0, 0.0, 0.0);

		ORIGIN
			glVertex3f(0.0, 0.0, 10.0);

		glEnd();

		glBegin(GL_POINTS);
		glVertex3f(lightpos[0], lightpos[1], lightpos[2]);
		glEnd();
		glUniformMatrix4fv(program_3d_polygon_mvp_l, 1, GL_FALSE, &mvp[0][0]);

		glm::vec3 d(
			sin(horizontal_angle),
			sin(vertical_angle),
			cos(horizontal_angle));

		glUseProgram(program_2d_image);
		if (!bullet_moving) {
			if (!click) {
				glBindTexture(GL_TEXTURE_2D, gun_texture);
				glBindVertexArray(gun_VAO);
				glBindBuffer(GL_ARRAY_BUFFER, gun_VBO);
				glDrawArrays(GL_QUADS, 0, 12);
			}
		}
		if (bullet_moving) {
			if (!click) {
				glBindTexture(GL_TEXTURE_2D, gun_texture);
				glBindVertexArray(gun_VAO);
				glBindBuffer(GL_ARRAY_BUFFER, gun_VBO);
				glDrawArrays(GL_QUADS, 0, 12);
			}
			if (click) {
				glBindTexture(GL_TEXTURE_2D, gun_texture);
				glBindVertexArray(gun_VAO);
				glBindBuffer(GL_ARRAY_BUFFER, gun_VBO);
				glDrawArrays(GL_QUADS, 0, 12);
			}
		}
		if (click) {
			if (!bullet_moving) {
				glBindTexture(GL_TEXTURE_2D, gun_2_texture);
				glBindVertexArray(gun_2_VAO);
				glBindBuffer(GL_ARRAY_BUFFER, gun_2_VBO);
				glDrawArrays(GL_QUADS, 0, 12);
			}
			bullet_moving = true;

			std::vector<glm::vec3> triangle_vec3s;
			std::vector<float> VERTEX;

			for (int i = 1; i <= monkey.vertices.size(); i++) {
				VERTEX.push_back(monkey.vertices[i - 1]);
				if (i % 3 == 0) {
					triangle_vec3s.push_back(glm::vec3(VERTEX[0], VERTEX[1], VERTEX[2]));
					VERTEX.clear();
					if (i % 9 == 0) {
						float ptr;
						if (glm::intersectRayTriangle(pos, d, triangle_vec3s[0], triangle_vec3s[1], triangle_vec3s[2], result, ptr)) {
						}
						triangle_vec3s.clear();
					}
				}
			}

		}
		click = false;

		glUseProgram(program_2d_polygon);
		glBegin(GL_LINES);
//        glUniform3f(program_2d_polygon_color_l, 0.0, 1.0, 0.0);
// Doesn't work, gets overwritten by next uniform call (must use vertex colors in retained mode)
		glVertex2f(-0.01, 0.0);
		glVertex2f(0.01, 0.0);
		glVertex2f(0.0, -0.01);
		glVertex2f(0.0, 0.01);
		glEnd();

		glUseProgram(program_2d_polygon);
		glUniform3f(program_2d_polygon_color_l, 1.0, 1.0, 0.0);
		glPointSize(point_size);
		glBegin(GL_POINTS);
		glVertex2f(bullet_x, f(bullet_x));
		glEnd();

		draw_text(string_format("Pos: %f, %f, %f", pos[0], pos[1], pos[2]), 100.f, D_HEIGHT - 100.f, 1.f, 1.0f, 1.0f, 1.0f, glyphs, text_VAO, text_VBO, ortho_mvp, program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l);
		glUniformMatrix4fv(program_2d_text_mvp_l, 1, GL_FALSE, &ortho_mvp[0][0]);

		glfwSwapBuffers(window);
	}
}

float f(float x) {
	return -1.02336 * x;
}

