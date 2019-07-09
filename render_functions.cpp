#include <cmath>
#include <algorithm>
#include <vector>
#include "render_headers.h"
using namespace std;

unsigned int length = 0;
unsigned int volume;

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

bool check_coord(double **z_buf, int x, int y, double z) {
	if (z > z_buf[y][x])
		return 1; //mean that we can draw
	return false;
}

void write_z_coord(double **z_buf, int x, int y, double z) {
	z_buf[y][x] = z;
}

double count_z_coord(int x, LINE *line) {
	if (0 == (line->two.x - line->one.x))
		return line->one.z;
	double div = (line->two.z - line->one.z) / (double)(line->two.x - line->one.x);
	double out_value = div * (double)(x - line->one.x) + line->one.z;
	free(line);
	return out_value;
}

int set_side(int side) {
	if (side > 0)
		side = 1;
	else
		side = -1;
	return side;
}

void plot(RGB *front_color, RGB *back_color, double	transparency) {
	//alpha-channel method
	back_color->red   = (unsigned char)((double)back_color->red   * (1.0 - transparency) + (double)front_color->red   * transparency);
	back_color->green = (unsigned char)((double)back_color->green * (1.0 - transparency) + (double)front_color->green * transparency);
	back_color->blue  = (unsigned char)((double)back_color->blue  * (1.0 - transparency) + (double)front_color->blue  * transparency);
}

void draw_line_WU(RGB **image, RGB *color, LINE *line) {
	if (line->one.y == line->two.y)      // "plain" line
		for (int x = line->one.x; x <= line->two.x; x++) {
			if (check_borders(line->one.y, x))
				image[line->one.y][x] = *color;
		}
	else if (line->one.x == line->two.x) // "plain" line
		for (int y = line->one.y; y <= line->two.y; y++) {
			if (check_borders(y, line->one.x))
				image[y][line->one.x] = *color;
		}
	else {
		if (check_borders(line->one.y, line->one.x))
			image[line->one.y][line->one.x] = *color;
		if (check_borders(line->two.y, line->two.x))
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
					if (check_borders(x, tmp - dy))
						plot(color, &image[x][tmp - dy], 1 - intens);
					if (check_borders(x, tmp - dy + 1))
						plot(color, &image[x][tmp - dy + 1], intens);
				}
				else {
					if (check_borders(tmp - dy, x))
						plot(color, &image[tmp - dy][x], 1 - intens);
					if (check_borders(tmp - dy + 1, x))
						plot(color, &image[tmp - dy + 1][x], intens);
				}
			}
			else if (pitch_f) {
				if (check_borders(x, tmp))
					plot(color, &image[x][tmp], 1 - intens);
				if (check_borders(x, tmp + 1))
					plot(color, &image[x][tmp + 1], intens);
			}
			else {
				if (check_borders(tmp, x))
					plot(color, &image[tmp][x], 1 - intens);
				if (check_borders(tmp + 1, x))
					plot(color, &image[tmp + 1][x], intens);
			}
			error += d_error;
		}
	}
	free(line);
}

VERTEX *read_vertex(FILE *in_obj, VERTEX *vertices, unsigned int counter, unsigned int zoom) {
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
			vertices[counter].x = (int)(atof(number) * zoom + 0.5);
			if (neg)
				vertices[counter].x *= -1;
		}
		else if (j == 1) {
			vertices[counter].y = (int)(atof(number) * zoom + 0.5);
			if (neg)
				vertices[counter].y *= -1;
		}
		else {
			vertices[counter].z = atof(number);
			if (neg)
				vertices[counter].z *= -1;
		}
	}
	top_x = max(top_x, vertices[counter].x);
	top_y = max(top_y, vertices[counter].y);
	bottom_x = min(bottom_x, vertices[counter].x);
	bottom_y = min(bottom_y, vertices[counter].y);
	return vertices;
}

VERTEX *read_obj(FILE *in_obj, unsigned int zoom) {
	VERTEX *vertices = (VERTEX*)malloc(sizeof(VERTEX) * 2);
	unsigned int counter = 1;
	char tmp = fgetc(in_obj);
	while ((tmp != EOF) && (0 != tmp - 'f')) {
		if (0 == (tmp - 'v')) {
			tmp = fgetc(in_obj);
			if (0 == (tmp - ' ')) {
				vertices = read_vertex(in_obj, vertices, counter - 1, zoom);
				vertices = (VERTEX*)realloc(vertices, (++counter) * sizeof(VERTEX));
			}
		}
		else
			tmp = fgetc(in_obj);
	}
	length = counter;
	return vertices;
}

void draw_outline_of_triangle(RGB **image, RGB *color, TRIANGLE *tgl) {
	draw_line_WU(image, color, set_vector(&tgl->one, &tgl->two));
	draw_line_WU(image, color, set_vector(&tgl->two, &tgl->three));
	draw_line_WU(image, color, set_vector(&tgl->three, &tgl->one));
	free(color);
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
				if (y >= 0 && x1 > 0 && y < HEIGHT && x1 < WIDTH)
					if (check_coord(z_buf, x1, y, z)) { //check z-buffer
						image[y][x1] = *color;
						write_z_coord(z_buf, x1, y, z);
					}
			}
			else { // for reverse rotation
				int y_r = fin_y - back_ctr;
				//check z-buffer
				if (y_r >= 0 && x1 > 0 && y_r < HEIGHT && x1 < WIDTH)
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
		draw_plain_triangle(image, color, tgl, z_buf, false, false);
	else if (tgl->one.y == tgl->two.y) {           // if it's an overturned "plain" triangle
		draw_plain_triangle(image, color, tgl, z_buf, true,  false);
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

unsigned int *read_vertices_of_polygon(FILE *obj) {
	unsigned int *out_val = NULL;
	char tmp = '1';
	for (int i = 0; 0 != tmp - '\n'; i++) {
		out_val = (unsigned int*)realloc(out_val, sizeof(unsigned int) * (i + 1));
		out_val[i] = 0;
		while (0 != (tmp = fgetc(obj)) - '/') {
			out_val[i] = out_val[i] * 10 + (tmp - '0');
		}
		out_val[i]--;
		while ((0 != (tmp = fgetc(obj)) - ' ') && (0 != tmp - '\n')) {}
		volume = i;
	}
	return out_val;
}

bool convex_corner(VERTEX *point, vector<VERTEX>* figure) {
	bool parity = true;
	for (unsigned int i = 0; i < figure->size(); i++) {
		int max_y = max((*figure)[i].y, (*figure)[(i + 1) % figure->size()].y);
		int min_y = min((*figure)[i].y, (*figure)[(i + 1) % figure->size()].y);
		int max_x = max((*figure)[i].x, (*figure)[(i + 1) % figure->size()].x);
		int min_x = min((*figure)[i].x, (*figure)[(i + 1) % figure->size()].x);
		if ((*figure)[i].y != (*figure)[(i + 1) % figure->size()].y) {
			//not consider horizontal lines
			if (point->y <= max_y && point->y > min_y) {
				//consider top vertex
				if (point->y == max_y)
					parity = !parity;
				//consider crossing of line
				else if (point->x < max_x) {
					double factor = (max_y - min_y) / (double)(max_x - min_x);
					double x = (min_x * factor - min_y + point->y) / factor;
					if (x < point->x)
						parity = !parity;
				}
			}
		}
	}
	return parity;
}

void draw_object(RGB **image, FILE *obj, VERTEX *vertices, double **z_buf, bool triangle_f) {
	char tmp = fgetc(obj);
	while (tmp != EOF) {
		if ((0 == tmp - 'f') && (0 == (tmp = fgetc(obj)) - ' ')) {
			unsigned int *vertices_places = read_vertices_of_polygon(obj);
			vector<VERTEX> polygon;
			//read_vertices of polygon
			for (unsigned int i = 0; i <= volume; i++)
				polygon.push_back(vertices[vertices_places[i]]);
			//triangulation by ear method
			for (int i = 0; polygon.size() > 2; i = i++ % polygon.size()) {
				VERTEX point = polygon[(i + 1) % polygon.size()];
				vector<VERTEX> figure = polygon;
				figure.erase(figure.begin() + (i + 1) % polygon.size());
				if (convex_corner(&point, &figure)) {
					TRIANGLE *tmp_t = set_triangle(&polygon[i], &polygon[(i + 1) % polygon.size()], &polygon[(i + 2) % polygon.size()]);
					if (triangle_f)
						draw_triangle(image, set_color(rand() % 256, rand() % 256, rand() % 256), tmp_t, z_buf);
					else
						draw_outline_of_triangle(image, set_color(rand() % 256, rand() % 256, rand() % 256), tmp_t);
					free(tmp_t);
					polygon = figure;
				}
			}
			tmp = fgetc(obj);
		}
		else
			tmp = fgetc(obj);
	}
}

bool check_borders(int x, int y) {
	if (y >= 0 && x >= 0 && y < WIDTH && x < HEIGHT)
		return true;
	return false;
}