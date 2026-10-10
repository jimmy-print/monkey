#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
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
#include <random>
#include <algorithm>
#include <set>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define START_WIREFRAME glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#define END_WIREFRAME glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#include <obj.h>
#include <utils.h>
#include <text.h>
#include <monkey.h>
#include <assert.h>

bool click = false;
bool freecam = true;
glm::vec2 result;

// https://stackoverflow.com/a/26221725/11772905
// CC0 1.0
template <typename... Args>
std::string string_format(const std::string &format, Args... args)
{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
	if (size_s <= 0)
	{
		throw std::runtime_error("Error during formatting.");
	}
	auto size = static_cast<size_t>(size_s);
	auto buf = std::make_unique<char[]>(size);
	std::snprintf(buf.get(), size, format.c_str(), args...);
	std::string ret_val = std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	std::replace(ret_val.begin(), ret_val.end(), '-', '_');
	return ret_val;
}

int init_time, now_time;
void log(std::string msg)
{
	now_time = std::chrono::duration_cast<std::chrono::milliseconds>(
				   std::chrono::system_clock::now().time_since_epoch())
				   .count();
	float diff = (float)(now_time - init_time) / 1000;
	std::cout << diff << " " << msg << "\n";
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && click == false)
	{
		click = true;
	}
	else
	{
		click = false;
	}
}

float wireframe = false;
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		freecam = !freecam;
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		wireframe = !wireframe;
	}
}

bool within_tenths(float x, float y);

float f(float x);

int main()
{
	init_time = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch())
					.count();

	srand(time(NULL));

	struct shared ss = get_shared_data("obj.obj");
	struct shared *s = &ss;

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	std::cout << "I'm apple machine" << std::endl;
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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
		glm::radians(FOV), (float)D_WIDTH / (float)D_HEIGHT, 0.1f, 100.0f);
	float initial_y = 1;
	glm::vec3 pos(-3, 2, 0);
	glm::mat4 view = glm::lookAt(pos,
								 glm::vec3(1, -0.2, 0),
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

	glm::vec3 P(-0.939192, 0.1, 1.107);  // Vector representing location of effector point
	float R = 0.08;  // Half the length of the cube surrounding the effector point
	// 3D polygons section
	GLuint program_3d_polygon = load_shader("shaders/3d_polygon_vs", "shaders/3d_polygon_fs");
	GLuint program_3d_polygon_mvp_l = glGetUniformLocation(program_3d_polygon, "mvp");

	float axis_len = 20.0f;
	float axis_height = 0.1f;
	float poly_vs[] = {
		// red and blue rectangle that specifies +x and +z axes respectively.
		0.0, 0.0, 0.0, 		    RED,
		0.0, axis_height, 0.0,      RED,
		axis_len, 0.0, 0.0,         RED,
		axis_len, 0.0, 0.0,         RED,
		axis_len, axis_height, 0.0, RED,
		0.0, axis_height, 0.0,      RED,

		0.0, 0.0, 0.0,		    BLUE,
		0.0, axis_height, 0.0,      BLUE,
		0.0, 0.0, axis_len,         BLUE,
		0.0, 0.0, axis_len,         BLUE,
		0.0, axis_height, axis_len, BLUE,
		0.0, axis_height, 0.0,      BLUE,

		P[0]-R/2, P[1]+R/2, P[2]-R/2, YELLOW,    // Cube representing space around the effector point.
		P[0]-R/2, P[1]-R/2, P[2]-R/2, YELLOW,
		P[0]+R/2, P[1]-R/2, P[2]-R/2, YELLOW,

		P[0]-R/2, P[1]+R/2, P[2]-R/2, YELLOW,
		P[0]+R/2, P[1]+R/2, P[2]-R/2, YELLOW,
		P[0]+R/2, P[1]-R/2, P[2]-R/2, YELLOW,

		P[0]-R/2, P[1]+R/2, P[2]+R/2, YELLOW,
		P[0]-R/2, P[1]-R/2, P[2]+R/2, YELLOW,
		P[0]+R/2, P[1]-R/2, P[2]+R/2, YELLOW,

		P[0]-R/2, P[1]+R/2, P[2]+R/2, YELLOW,
		P[0]+R/2, P[1]+R/2, P[2]+R/2, YELLOW,
		P[0]+R/2, P[1]-R/2, P[2]+R/2, YELLOW,


		P[0]-R/2, P[1]+R/2, P[2]-R/2, YELLOW,
		P[0]-R/2, P[1]+R/2, P[2]+R/2, YELLOW,
		P[0]-R/2, P[1]-R/2, P[2]-R/2, YELLOW,

		P[0]-R/2, P[1]-R/2, P[2]-R/2, YELLOW,
		P[0]-R/2, P[1]-R/2, P[2]+R/2, YELLOW,
		P[0]-R/2, P[1]+R/2, P[2]+R/2, YELLOW,

		P[0]+R/2, P[1]+R/2, P[2]-R/2, YELLOW,
  		P[0]+R/2, P[1]+R/2, P[2]+R/2, YELLOW,
  		P[0]+R/2, P[1]-R/2, P[2]-R/2, YELLOW,
  
  		P[0]+R/2, P[1]-R/2, P[2]-R/2, YELLOW,
  		P[0]+R/2, P[1]-R/2, P[2]+R/2, YELLOW,
  		P[0]+R/2, P[1]+R/2, P[2]+R/2, YELLOW,
	};
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(poly_vs), poly_vs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// image section
	GLuint program_3d_image = load_shader("shaders/3d_image_vs", "shaders/3d_image_fs");
	GLuint program_3d_image_mvp_l = glGetUniformLocation(program_3d_image, "mvp");
	float radius_of_map = 100.f;
	float image_vs[] = {
		-radius_of_map, 0.0, -radius_of_map,      0.0, 0.0, // lower left
		-radius_of_map, 0.0, radius_of_map,		  0.0, 5.0, // upper left. the 5.0 means something like 5 to 1 scaling of the floor image.
		radius_of_map, 0.0, -radius_of_map,		  5.0, 0.0, // lower right

		-radius_of_map, 0.0, radius_of_map,       0.0, 5.0, // upper left
		radius_of_map, 0.0, radius_of_map,	  5.0, 5.0, // upper right
		radius_of_map, 0.0, -radius_of_map,	  5.0, 0.0, // lower right
	};
	GLuint image_VAO, image_VBO;
	glGenVertexArrays(1, &image_VAO);
	glBindVertexArray(image_VAO);

	glGenBuffers(1, &image_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, image_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(image_vs), image_vs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
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
	unsigned char *data = stbi_load("Grass.jpg", &w, &h, &n, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		std::cout << "Failed to load Grass.jpg" << "\n";
	}
	stbi_image_free(data);



	GLuint program_2d_polygon = load_shader("shaders/2d_polygon_vs", "shaders/2d_polygon_fs");
	GLuint program_2d_polygon_color_l = glGetUniformLocation(program_2d_polygon, "color_l");
	// Load vs for Crosshair.
	float crosshair_vs[] = {
		0.0, 0.0,
		0.0, 0.05,
		0.0, 0.0,
		0.0, -0.05,
		0.0, 0.0,
		0.025, 0.0,
		0.0, 0.0,
		-0.025, 0.0,
	};
	GLuint poly2_VAO, poly2_VBO;
	glGenVertexArrays(1, &poly2_VAO);
	glBindVertexArray(poly2_VAO);
	glGenBuffers(1, &poly2_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, poly2_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair_vs), crosshair_vs, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);




	GLuint program_2d_image = load_shader("shaders/2d_image_vs", "shaders/2d_image_fs");

	std::vector<Monkey *> monkeys;
	int init_monkeys_num = 20;
	for (int i = 0; i < init_monkeys_num; i++)
	{
		monkeys.push_back(new Monkey(s, pos));
	}

	glm::vec3 d; // 3D unit direction vector
	// Converted from spherical coordinate system variables (horizontal_angle, vertical_angle)
	// Different from glm::vec3 direction, which is used to calculate the view matrix.

	glm::vec3 lightpos(0.0, 10.0, 0.0);

	float dy = 0;
	float gravity_const = 0.008;
	float jump_strength = 0.1;
	bool in_air = false;

	float slowdown_multiplier = 0.1;
	float actual_travel_speed = travel_speed;

	float bullet_x = 0.214;
	float point_size = 10;
	bool bullet_moving = false;
	float crouch = 0.3;

	float visibility = 5;
	// draw a line (shortest distance) between pos and monkey[*].
	// if the line's length is greater than 10, the monkey will not
	// rotate or move to the player.)

	glm::vec3 point(1, 1, 1);

	std::map<char, glyph> glyphs = init_glyphs("LiberationMono-Regular.ttf");
	configure_blending();
	GLuint text_VAO, text_VBO;
	get_vao_vbo(&text_VAO, &text_VBO);
	glm::mat4 ortho_mvp = get_mvp();
	GLuint program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l;
	get_shader("shaders/2d_text_vs", "shaders/2d_text_fs", &program_2d_text, &program_2d_text_mvp_l, &program_2d_text_color_l);

	int health = 100;
	int bullet_damage = 10;
	float bullet_animation_horizontal_speed = 0.05;

	int i = 0;
	int dd = 0;

	// Load spyder body here
	shared body_s = get_shared_data("body.obj");
	Model spyder_body(&body_s);

	// First leg: trochanter
	float trochanter_length = 0.3;
	// Since trochanter can only rotate in the xz plane from spyder_body,
	// it only has yaw.
	
	float trochanter_yaw = 1.226330217;//rad(-60);
	// in radians, where positive means that it rotates counter-clockwise viewed from top.
	// this is also absolute, relative to world pos.

	glm::vec3 trochanter_start_pos(-0.949193, 0.645508, 0.531212);
	glm::vec3 orig_trochanter_start_pos = glm::vec3(trochanter_start_pos[0], trochanter_start_pos[1], trochanter_start_pos[2]);
	glm::vec3 trochanter_end_pos(
		trochanter_start_pos[0] + trochanter_length * sin(trochanter_yaw),
		trochanter_start_pos[1],
		trochanter_start_pos[2] + trochanter_length * cos(trochanter_yaw));

	float trochanter_poly_vs[] = {
		trochanter_start_pos[0], trochanter_start_pos[1], trochanter_start_pos[2], RED,
		trochanter_end_pos[0], trochanter_end_pos[1], trochanter_end_pos[2], RED,
		trochanter_end_pos[0], trochanter_end_pos[1], trochanter_end_pos[2], RED,
	};
	GLuint trochanter_VAO, trochanter_VBO;
	glGenVertexArrays(1, &trochanter_VAO);
	glBindVertexArray(trochanter_VAO);

	glGenBuffers(1, &trochanter_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, trochanter_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(trochanter_poly_vs), trochanter_poly_vs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);



	// Second leg: femur
	float femur_length = 0.2;
	float femur_pitch = 3.95087; // in radians, where positive means that it rotates... the spyder body is on the player's left. then you look at the femur.
	glm::vec3 femur_start_pos(trochanter_end_pos[0], trochanter_end_pos[1], trochanter_end_pos[2]);

	glm::vec3 trochanter_dir_vector(
		trochanter_end_pos[0] - trochanter_start_pos[0],
		trochanter_end_pos[1] - trochanter_start_pos[1],
		trochanter_end_pos[2] - trochanter_start_pos[2]);
	// this vector is guaranteed to lay flat in the xz plane, i.e. not already be pointing 'upwards' or 'downwards' in the y direction.
	std::cout << trochanter_dir_vector[0] << " " << trochanter_dir_vector[1] << " " << trochanter_dir_vector[2] << "\n";

	// in conventional maths axes
	float femur_V_on_xz_plane_x = femur_length * cos(femur_pitch);
	float femur_V_on_xz_plane_z = femur_length * sin(femur_pitch);

	float femur_V_y = femur_V_on_xz_plane_x * sin(trochanter_yaw);
	float femur_V_x = femur_V_on_xz_plane_x * cos(trochanter_yaw);
	float femur_V_z = femur_V_on_xz_plane_z;
	glm::vec3 femur_dir_vector(femur_V_y, femur_V_z, femur_V_x);
	// so from maths, where it goes
	// X, Y, Z in order of right hand fingers, to
	// OpenGL's system, which is
	// Z, X, Y.
	// X=>Z, Y=>X, Z=>Y.

	glm::vec3 femur_end_pos(
		trochanter_end_pos[0] + femur_dir_vector[0],
		trochanter_end_pos[1] + femur_dir_vector[1],
		trochanter_end_pos[2] + femur_dir_vector[2]);

	float femur_poly_vs[] = {
		femur_start_pos[0], femur_start_pos[1], femur_start_pos[2], GREEN,
		femur_end_pos[0], femur_end_pos[1], femur_end_pos[2], GREEN,
		femur_end_pos[0], femur_end_pos[1], femur_end_pos[2], GREEN,
	};
	GLuint femur_VAO, femur_VBO;
	glGenVertexArrays(1, &femur_VAO);
	glBindVertexArray(femur_VAO);

	glGenBuffers(1, &femur_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, femur_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(femur_poly_vs), femur_poly_vs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);



	// Third leg: Tibia
	float tibia_length = 0.75;
	float tibia_pitch = -0.56371925;//rad(-70);
	glm::vec3 tibia_start_pos = glm::vec3(femur_end_pos);
	// all the below x, y, and z refer to axes as defined in math coordinate system, not OpenGL.
	float tibia_V_on_xz_plane_x = tibia_length * cos(tibia_pitch);
	float tibia_V_on_xz_plane_z = tibia_length * sin(tibia_pitch);
	float tibia_V_y = tibia_V_on_xz_plane_x * sin(trochanter_yaw);
	float tibia_V_x = tibia_V_on_xz_plane_x * cos(trochanter_yaw);
	float tibia_V_z = tibia_V_on_xz_plane_z;
	glm::vec3 tibia_dir_vector(tibia_V_y, tibia_V_z, tibia_V_x);
	glm::vec3 tibia_end_pos = tibia_start_pos + tibia_dir_vector;
	float tibia_poly_vs[] = {
		tibia_start_pos[0], tibia_start_pos[1], tibia_start_pos[2], BLUE,
		tibia_end_pos[0], tibia_end_pos[1], tibia_end_pos[2], BLUE,
		tibia_end_pos[0], tibia_end_pos[1], tibia_end_pos[2], BLUE,
	};
	GLuint tibia_VAO, tibia_VBO;
	glGenVertexArrays(1, &tibia_VAO);
	glBindVertexArray(tibia_VAO);

	glGenBuffers(1, &tibia_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, tibia_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tibia_poly_vs), tibia_poly_vs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	auto get_joint_angles = [&]() {
		// Currently, joint self-intersections and joint intersection
		// with spyder body are not considered. Effector positions with zero solutions
		// are also not handled (just crashes.)

		// Again, all the x,y,z used in this function for variable
		// names are their equivalents in the math coordinate axes.	

		float endx = P[2]; 
		float endy = P[0];
		float endz = P[1];

		std::cout << "End x: " << endx << " End y: " << endy << "End z: " << endz << "\n";

		float startx = trochanter_start_pos[2];
		float starty = trochanter_start_pos[0];
		float startz = trochanter_start_pos[1];

		float l = trochanter_length;
		float m = femur_length;
		float n = tibia_length;

		float phi = atan2(endy-starty,endx-startx);
		trochanter_end_pos[0] = trochanter_start_pos[0] + trochanter_length * sin(phi),
		trochanter_end_pos[1] = trochanter_start_pos[1],
		trochanter_end_pos[2] = trochanter_start_pos[2] + trochanter_length * cos(phi);


		std::cout << "trochanter_end_x " << trochanter_end_pos[2] << "y: " << trochanter_end_pos[0] << "\n";
		std::cout << "phi: " << phi << "\n";

		glm::vec3 femur_to_p(endx-trochanter_end_pos[2], endy-trochanter_end_pos[0], endz-trochanter_end_pos[1]);
		// direction vector from trochanter start to effector point.
		
		// with respect to, wrapped around the femur start point , NOT the origin.
		float P_on_xz_plane_y = 0;
		float P_on_xz_plane_x = sqrt(pow(femur_to_p[0], 2) + pow(femur_to_p[1], 2));
		float P_on_xz_plane_z = femur_to_p[2];

		// now solving for the pitches of the femur and tibia simply
		// becomes a question of 2 joints in 2D.

		std::cout << "turned x: " << P_on_xz_plane_x << ", turned z: " << P_on_xz_plane_z << "\n";

		auto F = [&](float gm) {
			return -P_on_xz_plane_z + m * sin(acos((P_on_xz_plane_x - n * cos(gm))/m)) + n * sin(gm);
		};
		float h = 0.000001;
		auto get_root_from_input_guess = [&](float guess, int iterations) {
			float F_output;
			float dF_output;
			float gm_guess = guess;
			for (int i = 0; i < iterations; i++) {
				F_output = F(gm_guess);
				dF_output = (F(gm_guess + h) - F(gm_guess)) / h;
				std::cout << "\t" << gm_guess << ", ";
				gm_guess = gm_guess - (F_output / dF_output); 
			}
			std::cout << "\n";
			return gm_guess;
		};
		std::vector<float> gms;
		for (float init_gm_guess = h; init_gm_guess < (2*PI - h); init_gm_guess += rad(5)) {
			float gm = get_root_from_input_guess(init_gm_guess, 20);
			if (gm > 0 && gm < 2 * PI) {
				gms.push_back(gm);
			}
		}
		std::sort(gms.begin(), gms.end());
		auto ip = std::unique(gms.begin(), gms.end(), [](float f, float g) { return std::abs(f - g) < 0.001; });
		gms.resize(std::distance(gms.begin(), ip));
		

		std::vector<std::vector<float>> angles;
		for (auto gm : gms) {
			float theta = acos((P_on_xz_plane_x - n*cos(gm))/m);
			angles.push_back(std::vector<float>{phi, theta, gm});
		}
		for (auto angle : angles) {
			std::cout << "Angle: " << angle[0] << " yaw, " << angle[1] << " pitch, " << angle[2] << " pitch\n";
		}
		std::cout << "\n";
		return angles;
	};
	std::vector<std::vector<float>> angles = get_joint_angles();

	while (!glfwWindowShouldClose(window))
	{
		i++;
		dd++;

		glClearColor(0.0, 0.5, 0.9, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();
		glfwGetCursorPos(window, &mouse_x, &mouse_y);
		glm::mat4 proj = glm::perspective(
			glm::radians(FOV), (float)D_WIDTH / (float)D_HEIGHT, 0.1f, 100.0f);
		horizontal_angle = mouse_speed * delay * float(D_WIDTH / 2 - mouse_x);
		vertical_angle = mouse_speed * delay * float(D_HEIGHT / 2 - mouse_y);
		horizontal_angle_deg = horizontal_angle * 180 / PI;
		vertical_angle_deg = vertical_angle * 180 / PI;
		d = glm::vec3(
			sin(horizontal_angle),
			sin(vertical_angle),
			cos(horizontal_angle));
		if (vertical_angle_deg < -90.0f)
		{
			mouse_y = (D_HEIGHT / 2) - (rad(-90.0f) / (mouse_speed * delay));
			vertical_angle = rad(-90.0f);
			glfwSetCursorPos(window, mouse_x, mouse_y);
		}
		if (vertical_angle_deg > 90.0f)
		{
			mouse_y = (D_HEIGHT / 2) - (rad(90.0f) / (mouse_speed * delay));
			vertical_angle = rad(90.0f);
			glfwSetCursorPos(window, mouse_x, mouse_y);
		}
		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		glm::vec3 direction(cos(vertical_angle) * sin(horizontal_angle),
							sin(vertical_angle),
							cos(vertical_angle) * cos(horizontal_angle));

		glm::vec3 right(sin(horizontal_angle - 3.14f / 2.0f),
						0,
						cos(horizontal_angle - 3.14f / 2.0f));
		glm::vec3 direction_without_pitch(sin(horizontal_angle), 0, cos(horizontal_angle));

		actual_travel_speed = travel_speed * slowdown_multiplier;

		if (freecam)
		{
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			{
				pos += direction * delay * actual_travel_speed;
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				pos -= direction * delay * actual_travel_speed;
			}
		}
		else
		{
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			{
				pos += direction_without_pitch * delay * actual_travel_speed;
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				pos -= direction_without_pitch * delay * actual_travel_speed;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			pos += right * delay * actual_travel_speed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			pos -= right * delay * actual_travel_speed;
		}

		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		{
			FOV = 20;
		}
		else
		{
			FOV = 70;
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
			angles = get_joint_angles();
			trochanter_yaw = angles[0][0];
			femur_pitch = angles[0][1];
			tibia_pitch = angles[0][2];
		}

		if (!freecam)
		{
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			{
				if (pos[1] == initial_y)
				{
					dy = jump_strength;
					in_air = true;
				}
			}
			dy -= gravity_const;

			pos[1] += dy;

			if (pos[1] <= initial_y)
			{
				dy = 0;
				pos[1] = initial_y;
				in_air = false;
			}
			if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !in_air)
			{
				pos[1] = initial_y - crouch;
			}
		}

		glm::vec3 up = glm::cross(right, direction);
		view = glm::lookAt(pos, pos + direction, up);

		mvp = proj * view * model;



		// Draw crosshair
		glUseProgram(program_2d_polygon);
		glBindVertexArray(poly2_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, poly2_VBO);
		glUniform3f(program_2d_polygon_color_l, 1.0, 1.0, 0.0);
		glLineWidth(1);
		glDrawArrays(GL_LINES, 0, 8);



		// Draw x-dir-specifying poly
		glUseProgram(program_3d_polygon);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glDrawArrays(GL_TRIANGLES, 0, 12);
		
		START_WIREFRAME
		glDrawArrays(GL_TRIANGLES, 12, 24);
		END_WIREFRAME
		glUniformMatrix4fv(program_3d_polygon_mvp_l, 1, GL_FALSE, &mvp[0][0]);

		
		// Control movement of spyder (manually for testing, with keys)
		if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
			//spyder_body.move_x(0.001);
			P[0] += 0.001;
		}
		if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
			//spyder_body.move_x(-0.001);
			P[0] -= 0.001;
		}
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
			//spyder_body.move_z(0.001);
			P[2] += 0.001;
		}
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
			//spyder_body.move_z(-0.001);
			P[2] -= 0.001;
		}
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
			P[1] += 0.001;
		}
		if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
			P[1] -= 0.001;
		}

		float poly_vs[] = {
		    // red and blue rectangle that specifies +x and +z axes respectively.
		    0.0, 0.0, 0.0, RED,
		    0.0, axis_height, 0.0, RED,
		    axis_len, 0.0, 0.0, RED,
		    axis_len, 0.0, 0.0, RED,
		    axis_len, axis_height, 0.0,      RED,
		    0.0, axis_height, 0.0,      RED,

		    0.0, 0.0, 0.0,      BLUE,
		    0.0, axis_height, 0.0,      BLUE,
		    0.0, 0.0, axis_len,      BLUE,
		    0.0, 0.0, axis_len,      BLUE,
		    0.0, axis_height, axis_len,      BLUE,
		    0.0, axis_height, 0.0,      BLUE,

		    // Cube representing space around the effector point.
		    P[0]-R/2, P[1]+R/2, P[2]-R/2, YELLOW,    
		    P[0]-R/2, P[1]-R/2, P[2]-R/2, YELLOW,
		    P[0]+R/2, P[1]-R/2, P[2]-R/2, YELLOW,

		    P[0]-R/2, P[1]+R/2, P[2]-R/2, YELLOW,
		    P[0]+R/2, P[1]+R/2, P[2]-R/2, YELLOW,
		    P[0]+R/2, P[1]-R/2, P[2]-R/2, YELLOW,

		    P[0]-R/2, P[1]+R/2, P[2]+R/2, YELLOW,
		    P[0]-R/2, P[1]-R/2, P[2]+R/2, YELLOW,
		    P[0]+R/2, P[1]-R/2, P[2]+R/2, YELLOW,

		    P[0]-R/2, P[1]+R/2, P[2]+R/2, YELLOW,
		    P[0]+R/2, P[1]+R/2, P[2]+R/2, YELLOW,
		    P[0]+R/2, P[1]-R/2, P[2]+R/2, YELLOW,


		    P[0]-R/2, P[1]+R/2, P[2]-R/2, YELLOW,
		    P[0]-R/2, P[1]+R/2, P[2]+R/2, YELLOW,
		    P[0]-R/2, P[1]-R/2, P[2]-R/2, YELLOW,

		    P[0]-R/2, P[1]-R/2, P[2]-R/2, YELLOW,
		    P[0]-R/2, P[1]-R/2, P[2]+R/2, YELLOW,
		    P[0]-R/2, P[1]+R/2, P[2]+R/2, YELLOW,

		    P[0]+R/2, P[1]+R/2, P[2]-R/2, YELLOW,
      		P[0]+R/2, P[1]+R/2, P[2]+R/2, YELLOW,
      		P[0]+R/2, P[1]-R/2, P[2]-R/2, YELLOW,
  
      		P[0]+R/2, P[1]-R/2, P[2]-R/2, YELLOW,
      		P[0]+R/2, P[1]-R/2, P[2]+R/2, YELLOW,
      		P[0]+R/2, P[1]+R/2, P[2]+R/2, YELLOW,
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(poly_vs), poly_vs, GL_STATIC_DRAW);


		if (wireframe) {
			START_WIREFRAME
		} else {
			END_WIREFRAME
		}
		spyder_body.draw(glm::vec3(0, 10, 0), lightpos, model, view, proj, glm::vec3(BLUE));
		END_WIREFRAME

		// Move trochanter according to spyder_body position.
		trochanter_start_pos[0] = orig_trochanter_start_pos[0] + spyder_body.displacement_x;
		trochanter_start_pos[1] = orig_trochanter_start_pos[1] + spyder_body.displacement_y;
		trochanter_start_pos[2] = orig_trochanter_start_pos[2] + spyder_body.displacement_z;
		
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			trochanter_yaw -= 0.001;
		} else if (glfwGetKey(window, GLFW_KEY_E)) {
			trochanter_yaw += 0.001;
		}

		glm::vec3 trochanter_end_pos(
			trochanter_start_pos[0] + trochanter_length * sin(trochanter_yaw),
			trochanter_start_pos[1],
			trochanter_start_pos[2] + trochanter_length * cos(trochanter_yaw));
		trochanter_poly_vs[0] = trochanter_start_pos[0];
		trochanter_poly_vs[1] = trochanter_start_pos[1];
		trochanter_poly_vs[2] = trochanter_start_pos[2];
		trochanter_poly_vs[6] = trochanter_end_pos[0];
		trochanter_poly_vs[7] = trochanter_end_pos[1];
		trochanter_poly_vs[8] = trochanter_end_pos[2];
		trochanter_poly_vs[12] = trochanter_end_pos[0];
		trochanter_poly_vs[13] = trochanter_end_pos[1];
		trochanter_poly_vs[14] = trochanter_end_pos[2];

		glBindBuffer(GL_ARRAY_BUFFER, trochanter_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(trochanter_poly_vs), trochanter_poly_vs, GL_STATIC_DRAW);

		// Move femur
		femur_start_pos[0] = trochanter_end_pos[0];
		femur_start_pos[2] = trochanter_end_pos[2];
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			femur_pitch += 0.001;
		} else if (glfwGetKey(window, GLFW_KEY_V)) {
			femur_pitch -= 0.001;
		}

		glm::vec3 trochanter_dir_vector(
			trochanter_end_pos[0] - trochanter_start_pos[0],
			trochanter_end_pos[1] - trochanter_start_pos[1],
			trochanter_end_pos[2] - trochanter_start_pos[2]);
		// this vector is guaranteed to lay flat in the xz plane, i.e. not already be pointing 'upwards' or 'downwards' in the y direction.
		//	std::cout << trochanter_dir_vector[0] << " " << trochanter_dir_vector[1] << " " << trochanter_dir_vector[2] <<  "\n";

    	// in conventional maths axes
    	float femur_V_on_xz_plane_x = femur_length * cos(femur_pitch);
    	float femur_V_on_xz_plane_z = femur_length * sin(femur_pitch);

    	float femur_V_y = femur_V_on_xz_plane_x * sin(trochanter_yaw);
    	float femur_V_x = femur_V_on_xz_plane_x * cos(trochanter_yaw);
    	float femur_V_z = femur_V_on_xz_plane_z;
    	glm::vec3 femur_dir_vector(femur_V_y, femur_V_z, femur_V_x);


		glm::vec3 femur_end_pos(
			trochanter_end_pos[0] + femur_dir_vector[0],
			trochanter_end_pos[1] + femur_dir_vector[1],
			trochanter_end_pos[2] + femur_dir_vector[2]);

		femur_poly_vs[0] = femur_start_pos[0];
		femur_poly_vs[1] = femur_start_pos[1];
		femur_poly_vs[2] = femur_start_pos[2];
		femur_poly_vs[6] = femur_end_pos[0];
		femur_poly_vs[7] = femur_end_pos[1];
		femur_poly_vs[8] = femur_end_pos[2];
		femur_poly_vs[12] = femur_end_pos[0];
		femur_poly_vs[13] = femur_end_pos[1];
		femur_poly_vs[14] = femur_end_pos[2];

		glBindBuffer(GL_ARRAY_BUFFER, femur_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(femur_poly_vs), femur_poly_vs, GL_STATIC_DRAW);

		float trochanter_poly_vs[] = {
			trochanter_start_pos[0],
			trochanter_start_pos[1],
			trochanter_start_pos[2],
			RED,
			trochanter_end_pos[0],
			trochanter_end_pos[1],
			trochanter_end_pos[2],
			RED,
			trochanter_end_pos[0],
			trochanter_end_pos[1],
			trochanter_end_pos[2],
			RED,
		};
		glBindBuffer(GL_ARRAY_BUFFER, trochanter_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(trochanter_poly_vs), trochanter_poly_vs, GL_STATIC_DRAW);

		// Draw trochanter
		glUseProgram(program_3d_polygon);
		glBindVertexArray(trochanter_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, trochanter_VBO);
		glLineWidth(5);
		glDrawArrays(GL_LINES, 0, 3);
		glUniformMatrix4fv(program_3d_polygon_mvp_l, 1, GL_FALSE, &mvp[0][0]);

		// Draw femur
		glUseProgram(program_3d_polygon);
		glBindVertexArray(femur_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, femur_VBO);
		glLineWidth(5);
		glDrawArrays(GL_LINES, 0, 3);
		glUniformMatrix4fv(program_3d_polygon_mvp_l, 1, GL_FALSE, &mvp[0][0]);

    	glm::vec3 tibia_start_pos = glm::vec3(femur_end_pos);
    	// all the below x, y, and z refer to axes as defined in math coordinate system, not OpenGL.
    	float tibia_V_on_xz_plane_x = tibia_length * cos(tibia_pitch);
    	float tibia_V_on_xz_plane_z = tibia_length * sin(tibia_pitch);
    	float tibia_V_y = tibia_V_on_xz_plane_x * sin(trochanter_yaw);
    	float tibia_V_x = tibia_V_on_xz_plane_x * cos(trochanter_yaw);
    	float tibia_V_z = tibia_V_on_xz_plane_z;
    	glm::vec3 tibia_dir_vector(tibia_V_y, tibia_V_z, tibia_V_x);
    	glm::vec3 tibia_end_pos = tibia_start_pos + tibia_dir_vector;
    	float tibia_poly_vs[] = {
    		tibia_start_pos[0], tibia_start_pos[1], tibia_start_pos[2], BLUE,
    		tibia_end_pos[0], tibia_end_pos[1], tibia_end_pos[2], BLUE,
    		tibia_end_pos[0], tibia_end_pos[1], tibia_end_pos[2], BLUE,
    	};
		// Draw tibia
		glUseProgram(program_3d_polygon);
		glBindVertexArray(tibia_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, tibia_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tibia_poly_vs), tibia_poly_vs, GL_STATIC_DRAW);
		glLineWidth(5);
		glDrawArrays(GL_LINES, 0, 3);
		glUniformMatrix4fv(program_3d_polygon_mvp_l, 1, GL_FALSE, &mvp[0][0]);



		// Draw grass ground
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(image_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, image_VBO);
		glUseProgram(program_3d_image);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glUniformMatrix4fv(program_3d_image_mvp_l, 1, GL_FALSE, &mvp[0][0]);



		// TODO: fix '-' glyph seems to render as a space
		// TODO: pack the arguments passed to draw_text that are the same every time into a struct
		int textbox_x = 200;
		draw_text("Spyder project: Robot arm simulation", textbox_x, D_HEIGHT - 50, 1, YELLOW, glyphs, text_VAO, text_VBO, ortho_mvp, program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l);
		draw_text("Red line points in +X, Blue line points in +Z.", textbox_x, D_HEIGHT - 75, 1, YELLOW, glyphs, text_VAO, text_VBO, ortho_mvp, program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l);
		draw_text("WASD for player movement, YGHJ for spyder_body movement (testing)", textbox_x, D_HEIGHT - 100, 1, YELLOW, glyphs, text_VAO, text_VBO, ortho_mvp, program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l);
		draw_text("Q,E to rotate trochanter, R,V to rotate femur up and down", textbox_x, D_HEIGHT - 125, 1, YELLOW, glyphs, text_VAO, text_VBO, ortho_mvp, program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l);

		draw_text(string_format("Player Position: (%f, %f, %f)", pos[0], pos[1], pos[2]), textbox_x, D_HEIGHT - 175, 1, YELLOW, glyphs, text_VAO, text_VBO, ortho_mvp, program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l);
		
		draw_text(string_format("Trochanter end pos: %f, %f, %f", trochanter_end_pos[0], trochanter_end_pos[1], trochanter_end_pos[2]), textbox_x + 700, D_HEIGHT - 150, 1, YELLOW, glyphs, text_VAO, text_VBO, ortho_mvp, program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l);
		draw_text(string_format("Tibia end pos: %f, %f, %f", tibia_end_pos[0], tibia_end_pos[1], tibia_end_pos[2]), textbox_x + 700, D_HEIGHT - 300, 1, YELLOW, glyphs, text_VAO, text_VBO, ortho_mvp, program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l);

		draw_text(string_format("Joint Angle: Trochanter %f yaw, Femur %f pitch", deg(trochanter_yaw), deg(femur_pitch)), textbox_x, D_HEIGHT - 300, 1, YELLOW, glyphs, text_VAO, text_VBO, ortho_mvp, program_2d_text, program_2d_text_mvp_l, program_2d_text_color_l);




		glfwSwapBuffers(window);
	}
}

bool within_tenths(float x, float y)
{
	return x <= y + 0.1 && x >= y - 0.1;
}

float f(float x)
{
	return -1.02336 * x;
}
