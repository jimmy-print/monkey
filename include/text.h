#ifndef TEXT_H
#define TEXT_H

#include <GL/glew.h>

struct glyph {
	GLuint texture;
	unsigned int bitmap_width;
	unsigned int bitmap_rows;
	int bitmap_left_offset;
	int bitmap_top_offset;
	unsigned int advance_x;
};

std::map<char, glyph> init_glyphs(std::string fontpath);
void configure_blending();
void get_vao_vbo(GLuint* VAO, GLuint* VBO);
glm::mat4 get_mvp();
void get_shader(std::string vspath, std::string fspath, GLuint* shader_out, GLuint* mvp_l_out, GLuint* color_l_out);
void draw_text(
	std::string text, 
	float x, float y, 
	float scale, 
	float r, float g, float b, 
	std::map<char, glyph> glyphs,
	GLuint VAO, GLuint VBO,
	glm::mat4 mvp,
	GLuint text_shader, GLuint mvp_l, GLuint color_l
);

#endif
