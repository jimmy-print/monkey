#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>

GLuint load_shader(std::string vs_filename, std::string fs_filename);
std::string get_file_str(std::string file);
bool click = false;
glm::vec2 result;

void print_vec3(glm::vec3 v)
{
	std::cout << v[0] << " " << v[1] << " " << v[2] << "\n";
}

float lastX = 1280 / 2.0;
float lastY = 720 / 2.0;
    float yaw = -90.f;
    float pitch = 0.0f;
    float firstMouse = true;
    	glm::vec3 pos(0, 0, 3);
    glm::vec3 looktowards(0, 0, 0);

void mouse_callback(GLFWwindow* w, double xpos, double ypos) {
    
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && click == false) {
		click = true;
	} else {
		click = false;
	}
}


int main()
{
    const int D_WIDTH = 1280;
	const int D_HEIGHT = 720;
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow *window = glfwCreateWindow(D_WIDTH, D_HEIGHT, "test ray triangle intersection", 0, 0);
	glfwMakeContextCurrent(window);
	glewInit();
	glEnable(GL_MULTISAMPLE);

	glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

    // 3D matrices, vecs and controls section

	float FOV = 70;
	glm::mat4 proj = glm::perspective(glm::radians(FOV), (float) D_WIDTH / (float) D_HEIGHT, 0.1f, 100.0f);

    glm::vec3 up(0, 1, 0);
	glm::mat4 view = glm::lookAt(pos,
				     looktowards,
				     up);
	glm::mat4 model(1.0f);
    glm::vec3 right = glm::normalize(glm::cross(up, looktowards));

    glm::vec3 direction;

direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
direction.y = sin(glm::radians(pitch));
direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));



    // Shader

    GLuint immediate = load_shader("immediatevs", "immediatefs");
	GLuint immediate_mvp_l = glGetUniformLocation(immediate, "mvp");
	
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float radius = 10.0f;

glm::mat4 view;
view = glm::lookAt(pos, looktowards, up);
const float cameraSpeed = 0.05f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        pos += cameraSpeed * looktowards;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        pos -= cameraSpeed * looktowards;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        pos -= glm::normalize(glm::cross(looktowards, up)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        pos += glm::normalize(glm::cross(looktowards, up)) * cameraSpeed;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
  
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;
    
    std::cout << "Pitch: " << pitch << " | Yaw: " << yaw << "\n";
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    looktowards = glm::normalize(direction);

    print_vec3(looktowards);



        glUseProgram(immediate);
        glBegin(GL_TRIANGLES);
        glVertex3f(2.0, 0.0, 0.0);
        glVertex3f(0.0, 2.0, 0.0);
        glVertex3f(0.0, 0.0, 2.0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(10.0, 0.0, 0.0);
        
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.0, 10.0, 0.0);
        
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.0, 0.0, 10.0);

        glEnd();
        glUniformMatrix4fv(immediate_mvp_l, 1, GL_FALSE, &glm::mat4(proj * view * model)[0][0]);

        glfwPollEvents();
        glfwSwapBuffers(window);
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
