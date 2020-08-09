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
	char outputImage[255];
	JSAMPARRAY inImg, outImg;

	if(argc < 2) {
		printf("Type Input Image Name\t---> ");
		scanf("%s", inputImage);
	} else {
		strcpy(inputImage, argv[1]);
	}
	sprintf(outputImage, "dawnSampling.jpg");

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

	out_width  = (int)(cinfo.image_width / 2);
	out_height = (int)(cinfo.image_height / 2);

	coutfo.image_width		= out_width;
	coutfo.image_height		= out_height;
	coutfo.input_components = 3;
	coutfo.in_color_space	= JCS_RGB;
	jpeg_set_defaults(&coutfo);
	jpeg_set_quality(&coutfo, 75, TRUE);
	jpeg_start_compress(&coutfo, TRUE);

	outImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		outImg[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width * 3);
		for(int j = 0; j < out_width; j++) {
			outImg[i][j * 3 + 0] = inImg[i * 2][j * 6 + 0];
			outImg[i][j * 3 + 1] = inImg[i * 2][j * 6 + 1];
			outImg[i][j * 3 + 2] = inImg[i * 2][j * 6 + 2];
		}
	}

	jpeg_write_scanlines(&coutfo, outImg, out_height);
	jpeg_finish_compress(&coutfo);

	jpeg_destroy_compress(&coutfo);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	for(int i = 0; i < out_height; i++) {
		free(outImg[i]);
		free(inImg[i]);
	}
	free(outImg);
	free(inImg);
	fclose(gp);

	printf("Done\n");
	return 0;
}