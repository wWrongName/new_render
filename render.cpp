#include "render_headers.h"
#define WIDTH 2048
#define HEIGHT 1152
#define BMP 0x4D42

using namespace std;

int main(int argc, char** argv)
{
	BMPFILEHEADER fh;
	BMPINFOHEADER ih;

	//set file header
	fh.bi_file_type = BMP;
	fh.bi_file_size = 7077942;
	fh.bi_file_res1 = 0;
	fh.bi_file_res2 = 0;
	fh.bi_off_bits  = 54;
	//set info header
	ih.bi_size   = 40;
	ih.bi_height = HEIGHT;
	ih.bi_width  = WIDTH;
	ih.bi_planes = 1;
	ih.bi_bit_count   = 24;
	ih.bi_compression = 0;
	ih.bi_size_image  = 7077888;
	ih.bi_x = 4724;
	ih.bi_y = 4724;
	ih.bi_clrs_used = 0;
	ih.bi_clrs_imp  = 0;

	//make raster and z-buffer
	RGB **image;
	double **z_buf;
	image = (RGB**)malloc(ih.bi_height * sizeof(RGB*));
	z_buf = (double**)malloc(ih.bi_height * sizeof(double*));
	for (int i = 0; i < ih.bi_height; i++) {
		image[i] = (RGB*)malloc(ih.bi_width * sizeof(RGB));
		z_buf[i] = (double*)malloc(ih.bi_width * sizeof(double));
	}
	//set background color and z-buffer value
	RGB *color = set_color(250, 250, 250);
	for (int i = 0; i < ih.bi_height; i++)
		for (int j = 0; j < ih.bi_width; j++) {
			image[i][j] = *color;
			z_buf[i][j] = -1.79769e+308; // (-1.79769e+308) is the smalest normalized number
		}
	free(color);

	//new output file (.bmp file)
	FILE *out = fopen("C:\\Users\\John\\Desktop\\output.bmp", "wb");
	if (out == NULL) {
		printf("It's not enough memory to create file.\n");
		return 1;
	}
	//input data (.obj file)
	FILE *obj  = fopen("C:\\Users\\John\\Desktop\\untitled.obj", "r");
	if (!obj) {
		printf("File does not exist.\n");
		return 1;
	}

	//create pointer for vertexes
	VERTEX *vertexes = NULL;
	vertexes = read_obj(obj);

	draw_object(image, obj, vertexes, z_buf);
	
	//write all data in the .bmp file
	fwrite(&fh, sizeof(BMPFILEHEADER), 1, out);
	fwrite(&ih, sizeof(BMPINFOHEADER), 1, out);
	for (int i = 0; i < ih.bi_height; i++) {
		for (int j = 0; j < ih.bi_width; j++) {
			fwrite(&image[i][j], sizeof(RGB), 1, out);
		}
	}

	for (int i = 0; i < ih.bi_height; i++) {
		free(image[i]);
	}
	free(image);
	fclose(out);
	fclose(obj);

	return 0;
}
