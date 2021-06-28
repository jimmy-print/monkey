#ifndef TRI_H
#define TRI_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct tri {
	float x0;
	float y0;
	float x1;
	float y1;
	float x2;
	float y2;

	struct tri *tri0;
	struct tri *tri1;
	struct tri *tri2;
};

void get_len_of_all_floats(struct tri *t, int *x);
struct tri *alloc();
void rearrange(float *arr, int len, struct tri *t, int *i);
void draw_arr(float *arr, int len);
void generate_tris(struct tri *t, struct tri bounding, int depth, int reverse_depth);
struct tri *get_tri_from_bounding(struct tri b);
void get_boundings_from_tri_and_bounding(struct tri outer_bounding, struct tri self, struct tri *p_bounding0, struct tri *p_bounding1, struct tri *p_bounding2);
void log_tris(struct tri *t, int layer);
void draw_tris(struct tri *t);
void draw_tri(struct tri *t, float *color);

#endif
