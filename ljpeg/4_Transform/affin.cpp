#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <jpeglib.h>
#include <math.h>
#include <iostream>

#define PI atan(1.0 / 4)

void inverse(double (*a)[3], double (*b)[3]);
double det(double (*a)[3]);

int main(int argc, char** argv) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_compress_struct coutfo;
	struct jpeg_error_mgr jerr;
	FILE* fp;
	FILE* gp;
	char inputImage[255];
	char outputImage[255] = "affin.jpg";
	double affin[3][3]	  = {{1.0, 0.0, 0.0}, {tan(-PI / 1.3), 1.0, 0.0}, {0.0, 0.0, 1.0}};
	JSAMPARRAY inImg, affinImg;

	if(argc < 2) {
		printf("Type Input Image Name\t---> ");
		scanf("%s", inputImage);
	} else {
		strcpy(inputImage, argv[1]);
	}

	jpeg_create_decompress(&cinfo);
	cinfo.err = jpeg_std_error(&jerr);
	if((fp = fopen(inputImage, "rb")) == NULL) {
		perror("fopen");
		exit(1);
	}

	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	inImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * cinfo.output_height);
	for(int i = 0; i < (int)cinfo.output_height; ++i) {
		inImg[i] = (JSAMPROW)calloc(sizeof(JSAMPLE), cinfo.output_width * cinfo.output_components);
	}
	while(cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, inImg + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);
	}
	fclose(fp);

	coutfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&coutfo);
	if((gp = fopen(outputImage, "wb")) == NULL) {
		perror("fopen");
		exit(1);
	}
	jpeg_stdio_dest(&coutfo, gp);

	/* 画像の保存 */
	coutfo.image_width		= cinfo.image_width;
	coutfo.image_height		= cinfo.image_height;
	coutfo.input_components = 3;
	coutfo.in_color_space	= JCS_RGB;
	jpeg_set_defaults(&coutfo);
	jpeg_set_quality(&coutfo, 75, TRUE);
	jpeg_start_compress(&coutfo, TRUE);
	affinImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * cinfo.image_height);
	for(int i = 0; i < (int)cinfo.image_height; i++) {
		affinImg[i] = (JSAMPROW)calloc(sizeof(JSAMPLE), cinfo.image_width * 3);
	}
	for(int i = 0; i < (int)cinfo.image_height; i++) {
		for(int j = 0; j < (int)cinfo.image_width; j++) {
			/* アフィン変換の実装 */
			int x, y;
			x = affin[0][0] * j + affin[0][1] * i + affin[0][2] * 1.0;
			y = affin[1][0] * j + affin[1][1] * i + affin[1][2] * 1.0;
			if(x > 0 && x < (int)cinfo.image_width && y > 0 && y < (int)cinfo.image_height) {
				affinImg[(int)y][(int)x * 3 + 0] = inImg[i][j * 3 + 0];
				affinImg[(int)y][(int)x * 3 + 1] = inImg[i][j * 3 + 1];
				affinImg[(int)y][(int)x * 3 + 2] = inImg[i][j * 3 + 2];
			}
		}
	}

	jpeg_write_scanlines(&coutfo, affinImg, cinfo.image_height);
	jpeg_finish_compress(&coutfo);

	jpeg_destroy_compress(&coutfo);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	for(int i = 0; i < (int)cinfo.image_height; i++) {
		free(affinImg[i]);
	}
	free(affinImg);
	fclose(gp);

	printf("Done\n");
	return 0;
}

void inverse(double (*a)[3], double (*b)[3]) {
	double d = det(a);
	b[0][0]	 = (a[1][1] * a[2][2] - a[1][2] * a[2][1]) / d;
	b[0][1]	 = -(a[0][1] * a[2][2] - a[0][2] * a[2][1]) / d;
	b[0][2]	 = (a[0][1] * a[1][2] - a[0][2] * a[1][1]) / d;
	b[1][0]	 = -(a[1][0] * a[2][2] - a[1][2] * a[2][0]) / d;
	b[1][1]	 = (a[0][0] * a[2][2] - a[0][2] * a[2][1]) / d;
	b[1][2]	 = -(a[0][0] * a[1][2] - a[0][2] * a[1][0]) / d;
	b[2][0]	 = (a[1][0] * a[2][1] - a[1][1] * a[2][0]) / d;
	b[2][1]	 = -(a[0][0] * a[2][1] - a[0][1] * a[2][0]) / d;
	b[2][2]	 = (a[0][0] * a[1][1] - a[0][1] * a[1][0]) / d;
}

double det(double (*a)[3]) {
	return a[0][0] * a[1][1] * a[2][2] + a[0][1] * a[1][2] * a[2][0] + a[0][2] * a[1][0] * a[2][1] - a[0][2] * a[1][1] * a[2][0] - a[0][1] * a[1][0] * a[2][2] - a[0][0] * a[1][2] * a[2][1];
}