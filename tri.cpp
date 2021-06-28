#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "tri.h"

// TODO: free memory

const int D_WIDTH = 1920;
const int D_HEIGHT = 1080;
float tri_color[] = {1.0, 1.0, 1.0};

void get_len_of_all_floats(struct tri *t, int *x)
{
	*x += 6;
	if (t->tri0 != NULL)
		get_len_of_all_floats(t->tri0, x);
	if (t->tri1 != NULL)
		get_len_of_all_floats(t->tri1, x);
	if (t->tri2 != NULL)
		get_len_of_all_floats(t->tri2, x);
}

struct tri *alloc()
{
	struct tri *t = (struct tri*) malloc(sizeof(struct tri));
	t->tri0 = NULL;
	t->tri1 = NULL;
	t->tri2 = NULL;

	return t;
}

void rearrange(float *arr, int len, struct tri *t, int *i)
{
	arr[*i + 0] = t->x0;
	arr[*i + 1] = t->y0;
	arr[*i + 2] = t->x1;
	arr[*i + 3] = t->y1;
	arr[*i + 4] = t->x2;
	arr[*i + 5] = t->y2;

	*i += 6;
	if (t->tri0 != NULL)
		rearrange(arr, len, t->tri0, i);
	if (t->tri1 != NULL)
		rearrange(arr, len, t->tri1, i);
	if (t->tri2 != NULL)
		rearrange(arr, len, t->tri2, i);
}

void draw_arr(float *arr, int len)
{
	glBegin(GL_TRIANGLES);

	for (int i = 0; i < len; i++) {
		if (i % 6 == 0) {
			glVertex2f(arr[i + 0], arr[i + 1]);
			glVertex2f(arr[i + 2], arr[i + 3]);
			glVertex2f(arr[i + 4], arr[i + 5]);
		}
	}

	glEnd();
}

void generate_tris(struct tri *t, struct tri bounding, int depth, int reverse_depth)
{
	if (depth == 0) {
		return;
	}

	struct tri *bounding_tri0 = alloc();
	struct tri *bounding_tri1 = alloc();
	struct tri *bounding_tri2 = alloc();

	get_boundings_from_tri_and_bounding(bounding, *t, bounding_tri0, bounding_tri1, bounding_tri2);

	struct tri *child_tri0 = alloc();
	struct tri *child_tri1 = alloc();
	struct tri *child_tri2 = alloc();

	child_tri0 = get_tri_from_bounding(*bounding_tri0);
	child_tri1 = get_tri_from_bounding(*bounding_tri1);
	child_tri2 = get_tri_from_bounding(*bounding_tri2);

	t->tri0 = child_tri0;
	t->tri1 = child_tri1;
	t->tri2 = child_tri2;

	generate_tris(t->tri0, *bounding_tri0, depth - 1, reverse_depth + 1);
	generate_tris(t->tri1, *bounding_tri1, depth - 1, reverse_depth + 1);
	generate_tris(t->tri2, *bounding_tri2, depth - 1, reverse_depth + 1);
}

struct tri *get_tri_from_bounding(struct tri b)
{
	struct tri *t = alloc();
	t->x0 = (b.x2 + b.x0) / 2;
	t->y0 = (b.y2 + b.y0) / 2;
	t->x1 = (b.x0 + b.x1) / 2;
	t->y1 = (b.y2 + b.y0) / 2;
	t->x2 = b.x0;
	t->y2 = b.y1;

	return t;
}

void get_boundings_from_tri_and_bounding(struct tri outer_bounding, struct tri self, struct tri *p_bounding0, struct tri *p_bounding1, struct tri *p_bounding2)
{
	p_bounding0->x0 = outer_bounding.x0;
	p_bounding0->y0 = outer_bounding.y0;
	p_bounding0->x1 = (outer_bounding.x0 + outer_bounding.x1) / 2;
	p_bounding0->y1 = (outer_bounding.y1 + outer_bounding.y0) / 2;
	p_bounding0->x2 = (outer_bounding.x2 + outer_bounding.x0) / 2;
	p_bounding0->y2 = (outer_bounding.y1 + outer_bounding.y0) / 2;

	p_bounding1->x0 = (outer_bounding.x0 + outer_bounding.x1) / 2;
	p_bounding1->y0 = (outer_bounding.y1 + outer_bounding.y0) / 2;
	p_bounding1->x1 = outer_bounding.x1;
	p_bounding1->y1 = outer_bounding.y1;
	p_bounding1->x2 = outer_bounding.x0;
	p_bounding1->y2 = outer_bounding.y2;

	p_bounding2->x0 = (outer_bounding.x2 + outer_bounding.x0) / 2;
	p_bounding2->y0 = (outer_bounding.y2 + outer_bounding.y0) / 2;
	p_bounding2->x1 = outer_bounding.x0;
	p_bounding2->y1 = outer_bounding.y2;
	p_bounding2->x2 = outer_bounding.x2;
	p_bounding2->y2 = outer_bounding.y2;
}

void log_tris(struct tri *t, int layer)
{
	printf("(%d) |%*s (%f, %f), (%f, %f), (%f, %f)\n", layer, layer, "", t->x0, t->y0, t->x1, t->y1, t->x2, t->y2);

	if ((t->tri0 == NULL) && (t->tri1 == NULL) && (t->tri2 == NULL)) {
		return;
	}

	log_tris(t->tri0, layer + 1);
	log_tris(t->tri1, layer + 1);
	log_tris(t->tri2, layer + 1);
}

void draw_tris(struct tri *t)
{
	draw_tri(t, tri_color);
	if ((t->tri0 == NULL) && (t->tri1 == NULL) && (t->tri2 == NULL)) {
		return;
	}

	draw_tris(t->tri0);
	draw_tris(t->tri1);
	draw_tris(t->tri2);
}

void draw_tri(struct tri *t, float *color)
{
//	glColor3f(color[0], color[1], color[2]);
	glBegin(GL_TRIANGLES);
	glVertex3f(t->x0, t->y0, 0);
	glVertex3f(t->x1, t->y1, 0);
	glVertex3f(t->x2, t->y2, 0);
	glEnd();
}
