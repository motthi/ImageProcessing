#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <jpeglib.h>

int main(int argc, char** argv) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_compress_struct coutfo;
	struct jpeg_error_mgr jerr;
	int out_height, out_width;
	FILE* fp;
	FILE* gp;
	char inputImage[255];
	char outputImage[255] = "output.jpg";

	/* 2値化用のルックアップテーブル */
	int LUT[256]  = {0};
	int threshold = 100;
	for(int i = threshold; i < 256; i++) {
		LUT[i] = 255;
	}

	if(argc == 1) {
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
	JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * cinfo.output_height);
	for(int i = 0; i < (int)cinfo.output_height; ++i) {
		buffer[i] = (JSAMPROW)calloc(sizeof(JSAMPLE), cinfo.output_width * cinfo.output_components);
	}
	while(cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);
	}
	fclose(fp);

	coutfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&coutfo);
	if((gp = fopen(outputImage, "wb")) == NULL) {
		perror("fopen");
		exit(1);
	}
	jpeg_stdio_dest(&coutfo, gp);

	out_width  = cinfo.image_width;
	out_height = cinfo.image_height;

	coutfo.image_width		= out_width;
	coutfo.image_height		= out_height;
	coutfo.input_components = 1;
	coutfo.in_color_space	= JCS_GRAYSCALE;
	jpeg_set_defaults(&coutfo);
	jpeg_set_quality(&coutfo, 75, TRUE);

	jpeg_start_compress(&coutfo, TRUE);

	/* 2値化の実行 */
	JSAMPARRAY img = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		img[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width);
		for(int j = 0; j < out_width; j++) {
			img[i][j] = LUT[(int)(0.3 * buffer[i][j * 3 + 0] + 0.59 * buffer[i][j * 3 + 1] + 0.11 * buffer[i][j * 3 + 2])];		   //red
		}
	}

	jpeg_write_scanlines(&coutfo, img, out_height);
	jpeg_finish_compress(&coutfo);

	jpeg_destroy_compress(&coutfo);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	for(int i = 0; i < out_height; i++) {
		free(img[i]);
	}
	free(img);
	fclose(gp);

	printf("Done\n");
	return 0;
}