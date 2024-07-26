#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <iostream>
#include <map>

#include <text.h>
#include <utils.h>

// TODO: add namespace

std::map<char, glyph> init_glyphs(std::string fontpath)
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cout << "freetype library init failure\n";
		abort();
	}
	FT_Face face;
	if (FT_New_Face(ft, fontpath.c_str(), 0, &face)) {
		std::cout << "font \"" << fontpath << "\" load failure\n";
		abort();
	}
	int font_size = 15;
	FT_Set_Pixel_Sizes(face, 0, font_size);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	std::map<char, glyph> glyphs;

	for (unsigned char c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "glyph \"" << c << "\" load failure\n";
			abort();
		}
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glyph g = {
			texture,
			face->glyph->bitmap.width, face->glyph->bitmap.rows,
			face->glyph->bitmap_left, face->glyph->bitmap_top,
			(unsigned int) face->glyph->advance.x
		};
		glyphs.insert(std::pair<char, glyph>(c, g));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return glyphs;
}

void configure_blending()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void get_vao_vbo(GLuint* VAO, GLuint* VBO)
{
	glGenVertexArrays(1, VAO);
	glGenBuffers(1, VBO);
	glBindVertexArray(*VAO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

glm::mat4 get_mvp()
{
	glm::mat4 projection = glm::ortho(0.0f, (float) D_WIDTH, 0.0f, (float) D_HEIGHT);
	return projection;
}

void get_shader(std::string vspath, std::string fspath, GLuint* shader_out, GLuint* mvp_l_out, GLuint* color_l_out)
{
	*shader_out = load_shader(vspath, fspath);
	*mvp_l_out = glGetUniformLocation(*shader_out, "mvp");
	*color_l_out = glGetUniformLocation(*shader_out, "color_l");
}

void draw_text(
	std::string text,
	float x, float y,
	float scale,
	float r, float g, float b,
	std::map<char, glyph> glyphs,
	GLuint VAO, GLuint VBO,
	glm::mat4 mvp,
	GLuint text_shader, GLuint mvp_l, GLuint color_l)
{
	for (auto c : text) {
		glUseProgram(text_shader);
		glUniform3f(color_l, r, g, b);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO);

		glyph g = glyphs[c];

		float xx = x + g.bitmap_left_offset * scale;
		float yy = y - (g.bitmap_rows - g.bitmap_top_offset) * scale;
		float w = g.bitmap_width * scale;
		float h = g.bitmap_rows * scale;

		float vs[24] = {
			xx, yy + h,		0.0f, 0.0f,
			xx, yy,			0.0f, 1.0f,
			xx + w, yy,		1.0f, 1.0f,

			xx, yy + h,		0.0f, 0.0f,
			xx + w, yy,		1.0f, 1.0f,
			xx + w, yy + h,	1.0f, 0.0f,
		};

		glBindTexture(GL_TEXTURE_2D, g.texture);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vs), vs);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		x += (g.advance_x >> 6) * scale;
	}
	glUniformMatrix4fv(mvp_l, 1, GL_FALSE, &mvp[0][0]);

}
