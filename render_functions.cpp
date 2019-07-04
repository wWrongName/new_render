#include <cmath>
#include <algorithm>
#include "render_headers.h"
using namespace std;

VERTEX *set_vertex(int x, int y, double z) {
	VERTEX *out_vertex = (VERTEX*)malloc(sizeof(VERTEX));
	out_vertex->x = x;
	out_vertex->y = y;
	out_vertex->z = z;
	return out_vertex;
}

LINE *set_vector(VERTEX *one, VERTEX *two) {
	LINE *out_vector = (LINE*)malloc(sizeof(LINE));
	out_vector->one = *one;
	out_vector->two = *two;
	return out_vector;
}

TRIANGLE *set_triangle(VERTEX *one, VERTEX *two, VERTEX *three) {
	TRIANGLE *out_triangle = (TRIANGLE*)malloc(sizeof(TRIANGLE));
	out_triangle->one = *one;
	out_triangle->two = *two;
	out_triangle->three = *three;
	return out_triangle;
}

RGB *set_color(unsigned char R, unsigned char G, unsigned char B) {
	RGB *clr_of_pix = (RGB*)malloc(sizeof(RGB));
	clr_of_pix->blue = B;
	clr_of_pix->green = G;
	clr_of_pix->red = R;
	return clr_of_pix;
}

bool check_coord(double **z_buf, int x, int y, int z) {
	if (z > z_buf[y][x])
		return 1; //mean that we can draw
	return 0;
}

void write_z_coord(double **z_buf, int x, int y, int z) {
	z_buf[y][x] = z;
}

double count_z_coord(int x, LINE *line) {
	if (0 == (line->two.x - line->one.x))
		return line->one.z;
	double div = (line->two.z - line->one.z) / (double)(line->two.x - line->one.x);
	return div * (double)(x - line->one.x) + line->one.z;
}

int set_side(int side) {
	if (side > 0)
		side = 1;
	else
		side = -1;
	return side;
}

void plot(RGB *front_color, RGB *back_color, double	transparency) {
	back_color->red   = (unsigned char)((double)back_color->red   * (1.0 - transparency) + (double)front_color->red   * transparency);
	back_color->green = (unsigned char)((double)back_color->green * (1.0 - transparency) + (double)front_color->green * transparency);
	back_color->blue  = (unsigned char)((double)back_color->blue  * (1.0 - transparency) + (double)front_color->blue  * transparency);
}

void draw_line_WU(RGB **image, RGB *color, LINE *line) {
	if (line->one.y == line->two.y)      // "plain" line
		for (int x = line->one.x; x <= line->two.x; x++)
			image[line->one.y][x] = *color;

	else if (line->one.x == line->two.x) // "plain" line
		for (int y = line->one.y; y <= line->two.y; y++)
			image[y][line->one.x] = *color;

	else {
		image[line->one.y][line->one.x] = *color;
		image[line->two.y][line->two.x] = *color;
		bool pitch_f = 0, left_f = 0;
		int dx = line->two.x - line->one.x;
		int dy = line->two.y - line->one.y;
		int side = line->two.y - line->one.y;
		int side_x = line->two.x - line->one.x;
		double error = 0;

		side = set_side(side);
		side_x = set_side(side_x);

		if (side * (line->two.x - line->one.x) > 0) { // requirement for left slant
			std::swap(line->one.y, line->two.y);
			left_f = 1;
		}

		if (abs(dx) < abs(dy)) {
			swap(line->one.x, line->two.y);
			swap(line->two.x, line->one.y);
			swap(dx, dy);
			pitch_f = 1;
		}

		double d_error = (float)dy / dx * side_x;
		error = line->one.y + d_error;

		for (int x = line->one.x + side_x; x != line->two.x; x += side_x) {
			int tmp = (int)error;
			double intens = error - tmp;
			if (left_f) {
				if (pitch_f) {
					plot(color, &image[x][tmp - dy], 1 - intens);
					plot(color, &image[x][tmp - dy + 1], intens);
				}
				else {
					plot(color, &image[tmp - dy][x], 1 - intens);
					plot(color, &image[tmp - dy + 1][x], intens);
				}
			}
			else if (pitch_f) {
				plot(color, &image[x][tmp], 1 - intens);
				plot(color, &image[x][tmp + 1], intens);
			}
			else {
				plot(color, &image[tmp][x], 1 - intens);
				plot(color, &image[tmp + 1][x], intens);
			}
			error += d_error;
		}
	}
}

VERTEX *read_vertex(FILE *in_obj, VERTEX *vertexes, unsigned int counter) {
	char number[8];
	char tmp;
	bool neg;
	for (int j = 0; j < 3; j++) {
		neg = 0;
		tmp = fgetc(in_obj);
		for (int i = 0; i < 8; i++) {
			if (0 != (tmp - '-')) {
				number[i] = tmp;
			}
			else {
				neg = 1;
				i--;
			}
			tmp = fgetc(in_obj);
		}
		// 250 is the scale (250 to 1)
		if (j == 0) {
			vertexes[counter].x = (int)(atof(number) * 250);
			if (neg)
				vertexes[counter].x *= -1;
		}
		else if (j == 1) {
			vertexes[counter].y = (int)(atof(number) * 250);
			if (neg)
				vertexes[counter].y *= -1;
		}
		else {
			vertexes[counter].z = (double)((int)(atof(number) * 250));
			if (neg)
				vertexes[counter].z *= -1;
		}
	}
	return vertexes;
}

VERTEX *read_obj(FILE *in_obj) {
	VERTEX *vertexes = (VERTEX*)malloc(sizeof(VERTEX) * 2);
	unsigned int counter = 1;
	char tmp = fgetc(in_obj);
	while ((tmp != EOF) && (0 != tmp - 'f')) {
		if (0 == (tmp - 'v')) {
			tmp = fgetc(in_obj);
			if (0 == (tmp - ' ')) {
				vertexes = read_vertex(in_obj, vertexes, counter - 1);
				vertexes = (VERTEX*)realloc(vertexes, (++counter) * sizeof(VERTEX));
			}
		}
		else
			tmp = fgetc(in_obj);
	}
	return vertexes;
}

void draw_outline_of_triangle(RGB **image, RGB *color, TRIANGLE *tgl) {
	draw_line_WU(image, color, set_vector(&tgl->one, &tgl->two));
	draw_line_WU(image, color, set_vector(&tgl->two, &tgl->three));
	draw_line_WU(image, color, set_vector(&tgl->three, &tgl->one));
}

void draw_plain_triangle(RGB **image, RGB *color, TRIANGLE *tgl, double **z_buf, bool flip_f, bool free_f) {
	if (flip_f == 1) {
		swap(tgl->one.y, tgl->three.y);
		tgl->two.y = tgl->one.y;
		tgl = sort_coords(tgl);
	}
	float delta_left  = (tgl->three.x - tgl->one.x) / (float)(tgl->three.y - tgl->one.y);
	float delta_right = (tgl->two.x - tgl->one.x)   / (float)(tgl->two.y - tgl->one.y);
	int fin_y = tgl->three.y;
	int back_ctr = -1;                 //for flip again
	for (int y = tgl->one.y; y <= fin_y; y++) {
		back_ctr++;
		int x1 = (int)((y - tgl->one.y) * delta_left  + tgl->one.x);
		int x2 = (int)((y - tgl->one.y) * delta_right + tgl->one.x);
		double z1 = count_z_coord(x1, set_vector(&tgl->one, &tgl->three));
		double z2 = count_z_coord(x2, set_vector(&tgl->one, &tgl->two));
		if (x1 > x2) {
			swap(x1, x2);
		}
		VERTEX *one = set_vertex(x1, y, z1);
		VERTEX *two = set_vertex(x2, y, z2);
		//draw line between x1 and x2
		for (; x1 <= x2; x1++) {
			double z = count_z_coord(x1, set_vector(one, two));
			if (flip_f == 0) {
				if (y > 0 && x1 > 0)
					if (check_coord(z_buf, x1, y, z)) { //check z-buffer
						image[y][x1] = *color;
						write_z_coord(z_buf, x1, y, z);
					}
			}
			else { // for reverse rotation
				int y_r = fin_y - back_ctr;
				//check z-buffer
				if (y_r > 0 && x1 > 0)
					if (check_coord(z_buf, x1, y_r, z)){
						image[y_r][x1] = *color;
						write_z_coord(z_buf, x1, y_r, z);
					}
			}
		}
		free(one);
		free(two);
	}
	if (free_f)
		free(tgl);
 }

TRIANGLE *sort_coords(TRIANGLE *tgl) {
	VERTEX tmp;
	if (tgl->one.y > tgl->two.y) {
		tmp = tgl->one;
		tgl->one = tgl->two;
		tgl->two = tmp;
	}
	if (tgl->one.y > tgl->three.y) {
		tmp = tgl->one;
		tgl->one = tgl->three;
		tgl->three = tmp;
	}
	if (tgl->two.y > tgl->three.y) {
		tmp = tgl->two;
		tgl->two = tgl->three;
		tgl->three = tmp;
	}
	return tgl;
}

void draw_triangle(RGB **image, RGB *color, TRIANGLE *tgl, double **z_buf) {
	tgl = sort_coords(tgl);
	if (tgl->three.y == tgl->two.y)                // if it's a "plain" triangle
		draw_plain_triangle(image, color, tgl, z_buf, 0, 0);
	else if (tgl->one.y == tgl->two.y) {           // if it's an overturned "plain" triangle
		draw_plain_triangle(image, color, tgl, z_buf, 1, 0);
	}
	else {// if it's a triangle with different "Y" coordinates of vertices 
		/* we must divide a triangle into two "plain" triangles
		i.e. we must find vertex on the longest line*/
		double div  = (tgl->three.x - tgl->one.x) / (double)(tgl->three.y - tgl->one.y);
		double div2 = (tgl->three.z - tgl->one.z) / (double)(tgl->three.x - tgl->one.x);
		int    mid_x = (int)((tgl->two.y - tgl->one.y) * div + tgl->one.x);
		double mid_z = (mid_x - tgl->one.x) * div2 + tgl->one.z;
		//draw 2 triangle
		VERTEX *new_vertex = set_vertex(mid_x, tgl->two.y, mid_z);
		draw_plain_triangle(image, color, set_triangle(&tgl->one,  &tgl->two, new_vertex),  z_buf, 0, 1);
		draw_plain_triangle(image, color, set_triangle(new_vertex, &tgl->two, &tgl->three), z_buf, 1, 1);
		free(new_vertex);
	}
	free(color);
}

unsigned int read_arr_pos(FILE *obj) {
	unsigned int out_val = 0;
	char tmp;
	while (0 != (tmp = fgetc(obj)) - '/') {
		out_val = out_val * 10 + (tmp - '0');
	}
	while ((0 != (tmp = fgetc(obj)) - ' ') && (0 != tmp - '\n')) {}
	return out_val;
}

void draw_object(RGB **image, FILE *obj, VERTEX *vertexes, double **z_buf) {
	char tmp = fgetc(obj);
	while (tmp != EOF) {
		if ((0 == tmp - 'f') && (0 == (tmp = fgetc(obj)) - ' ')) {
			unsigned int first_v  = read_arr_pos(obj) - 1;
			unsigned int second_v = read_arr_pos(obj) - 1;
			unsigned int third_v  = read_arr_pos(obj) - 1;
			TRIANGLE tmp_t;
			tmp_t.one.x   = vertexes[first_v].x;
			tmp_t.one.y   = vertexes[first_v].y;
			tmp_t.one.z   = vertexes[first_v].z;
			tmp_t.two.x   = vertexes[second_v].x;
			tmp_t.two.y   = vertexes[second_v].y;
			tmp_t.two.z   = vertexes[second_v].z;
			tmp_t.three.x = vertexes[third_v].x;
			tmp_t.three.y = vertexes[third_v].y;
			tmp_t.three.z = vertexes[third_v].z;
			draw_triangle(image, set_color(rand() % 255, rand() % 255, rand() % 255), &tmp_t, z_buf);
			tmp = fgetc(obj);
		}
		else
			tmp = fgetc(obj);
	}
}