#pragma once
#include <iostream>

#pragma pack(push, 1)
typedef struct {
	unsigned short bi_file_type;
	unsigned int bi_file_size;
	unsigned short bi_file_res1;
	unsigned short bi_file_res2;
	unsigned int bi_off_bits;
} BMPFILEHEADER;
#pragma pack(pop)

#pragma pack(push, 1)        
typedef struct BMP {
	unsigned int bi_size;
	int bi_width;
	int bi_height;
	unsigned short bi_planes;
	unsigned short bi_bit_count;
	unsigned int bi_compression;
	unsigned int bi_size_image;
	int bi_x;
	int bi_y;
	unsigned int bi_clrs_used;
	unsigned int bi_clrs_imp;
} BMPINFOHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct COLOR_OF_PIXEL {
	unsigned char blue;
	unsigned char green;
	unsigned char red;
} RGB;
#pragma pack(pop)

typedef struct VERTEX_COORDS {
	int x;
	int y;
	double z;
} VERTEX;

typedef struct LINE_COORDS {
	VERTEX one;
	VERTEX two;
} LINE;

typedef struct TRGL_COORDS {
	VERTEX one;
	VERTEX two;
	VERTEX three;
} TRIANGLE;

void draw_plain_triangle(RGB**, RGB*, TRIANGLE*, double**, bool, bool);
void write_z_coord(double **z_buf, int x, int y, int z);
void draw_triangle(RGB**, RGB*, TRIANGLE*, double**);
void draw_object(RGB**, FILE*, VERTEX*, double**);
void draw_line_WU(RGB**, RGB*, LINE*);
void plot(RGB*, RGB*, double);

RGB *set_color(unsigned char, unsigned char, unsigned char);

bool check_coord(double **z_buf, int x, int y, int z);

TRIANGLE *set_triangle(VERTEX*, VERTEX*, VERTEX*);
TRIANGLE *sort_coords(TRIANGLE*);

VERTEX *read_vertex(FILE*, VERTEX*, unsigned int);
VERTEX *set_vertex(int, int, double);
VERTEX *read_obj(FILE*);

double count_z_coord(int x, LINE *line);

LINE *set_vector(VERTEX*, VERTEX*);

unsigned int read_arr_pos(FILE*);

int set_side(int);