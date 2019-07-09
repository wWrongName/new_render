#include <string>
#include "render_headers.h"
#define BMP 0x4D42
using namespace std;

int top_x = -2147483647, bottom_x = 2147483647;
int top_y = -2147483647, bottom_y = 2147483647;

int main(int argc, char** argv)
{
	char input_text[100];
	std::cout << "Write .obj file path (C:\\...) and type ENTER: ";
	std::cin.getline(input_text, 100);
	//input data (.obj file)
	FILE *obj = fopen(input_text, "r");
	if (!obj) {
		printf("File does not exist.\n");
		return 1;
	}
	std::cout << "Write .bmp file path and type ENTER: ";
	std::cin.getline(input_text, 100);
	//new output file (.bmp file)
	FILE *out = fopen(input_text, "wb");
	if (out == NULL) {
		printf("It's not enough memory to create file.\n");
		fclose(obj);
		return 1;
	}

	BMPFILEHEADER fh;
	BMPINFOHEADER ih;
	//set file header
	fh.bi_file_type = BMP;
	fh.bi_file_size = 7077942;
	fh.bi_file_res1 = 0;
	fh.bi_file_res2 = 0;
	fh.bi_off_bits = 54;
	//set info header
	ih.bi_size = 40;
	ih.bi_height = HEIGHT;
	ih.bi_width = WIDTH;
	ih.bi_planes = 1;
	ih.bi_bit_count = 24;
	ih.bi_compression = 0;
	ih.bi_size_image = 7077888;
	ih.bi_x = 4724;
	ih.bi_y = 4724;
	ih.bi_clrs_used = 0;
	ih.bi_clrs_imp = 0;

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
			// (-1.79769e+308) is the smalest normalized number
			z_buf[i][j] = -1.79769e+308;
		}
	free(color);

	unsigned int zoom = 250;
	cout << "Choose scale (for example 300) or type 'default' (default scale 250:1)\n";
	std::cin.getline(input_text, 100);
	if (strcmp(input_text, "default"))
		zoom = atoi(input_text);
	//create pointer for vertexes
	VERTEX *vertices = NULL;
	vertices = read_obj(obj, zoom);
	//alignment
	int offset_x = MID_WIDTH - (top_x + bottom_x) / 2 ;
	int offset_y = MID_HEIGHT - (top_y + bottom_y) / 2;
	for (unsigned int i = 0; i < length; i++) {
		vertices[i].x += offset_x;
		vertices[i].y += offset_y;
	}

	string key_word;
	cout << "Choose mode: f (draw object's frame) or s (draw object's surface)\n";
	getline(cin, key_word);
	if (key_word == "f")
		draw_object(image, obj, vertices, z_buf, false);
	else if (key_word == "s")
		draw_object(image, obj, vertices, z_buf, true);
	else {
		std::cout << "Invalid input.";
		for (int i = 0; i < ih.bi_height; i++) {
			free(image[i]);
		}
		free(image);
		fclose(out);
		fclose(obj);
		return 1;
	}
	
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
