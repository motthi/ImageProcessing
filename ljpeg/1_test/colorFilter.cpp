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
	//printf("%d\t%d\t%d\n", cinfo.image_width, cinfo.image_height, cinfo.num_components);

	jpeg_start_decompress(&cinfo);
	JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * cinfo.output_height);
	for(int i = 0; i < cinfo.output_height; ++i) {
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

	coutfo.image_width	   = out_width;
	coutfo.image_height	   = out_height;
	coutfo.input_components = cinfo.num_components;
	coutfo.in_color_space   = JCS_RGB;
	jpeg_set_defaults(&coutfo);
	jpeg_set_quality(&coutfo, 75, TRUE);

	jpeg_start_compress(&coutfo, TRUE);

	JSAMPARRAY img = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		img[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * 3 * out_width);
		for(int j = 0; j < out_width; j++) {
			img[i][j * 3 + 0] = buffer[i][j*3+0];	//red
			img[i][j * 3 + 1] = buffer[i][j*3+1];	//green
			img[i][j * 3 + 2] = buffer[i][j*3+2];	//blue
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