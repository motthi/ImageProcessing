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
	//double gaussianFilter[3][3]		  = {{1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}, {2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0}, {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}};
	//double sobelFilter_side[3][3]	  = {{1.0, 2.0, 1.0}, {0.0, 0.0, 0.0}, {-1.0, -2.0, -1.0}};
	//double sobelFilter_vertical[3][3] = {{1.0, 0.0, -1.0}, {2.0, 0.0, -2.0}, {1.0, 0.0, -1.0}};
	double laplacianFilter3[3][3] = {{1.0, 1.0, 1.0}, {1.0, -8.0, 1.0}, {1.0, 1.0, 1.0}};
	double laplacianFilter5[5][5] = {{0.0, 0.0, 1.0, 0.0, 0.0}, {0.0, 1.0, 2.0, 1.0, 0.0}, {1.0, 2.0, -16.0, 2.0, 1.0}, {0.0, 1.0, 2.0, 1.0}, {0.0, 0.0, 1.0, 0.0, 0.0}};
	double laplacianFilter7[7][7] = {
		{1.0, 3.0, 4.0, 4.0, 4.0, 3.0, 1.0},
		{3.0, 4.0, 3.0, 0.0, 3.0, 4.0, 3.0},
		{4.0, 3.0, -9.0, -17.0, -9.0, 3.0, 4.0},
		{4.0, 0.0, -17.0, -30.0, -17.0, 0.0, 4.0},
		{4.0, 3.0, -9.0, -17.0, -9.0, 3.0, 4.0},
		{3.0, 4.0, 3.0, 0.0, 3.0, 4.0, 3.0},
		{1.0, 3.0, 4.0, 4.0, 4.0, 3.0, 1.0}};
	double laplacianFilter9[9][9] = {
		{0.0, 1.0, 1.0, 2.0, 2.0, 2.0, 1.0, 1.0, 0.0},
		{1.0, 2.0, 4.0, 5.0, 5.0, 5.0, 4.0, 2.0, 1.0},
		{1.0, 4.0, 5.0, 3.0, 0.0, 3.0, 5.0, 4.0, 1.0},
		{2.0, 5.0, 3.0, -12.0, -24.0, -12.0, 3.5, 5.0, 2.0},
		{2.0, 5.0, 0.0, -24.0, -40.0, -24.0, 0.0, 5.0, 2.0},
		{2.0, 5.0, 3.0, -12.0, -24.0, -12.0, 3.5, 5.0, 2.0},
		{1.0, 4.0, 5.0, 3.0, 0.0, 3.0, 5.0, 4.0, 1.0},
		{1.0, 2.0, 4.0, 5.0, 5.0, 5.0, 4.0, 2.0, 1.0},
		{0.0, 1.0, 1.0, 2.0, 2.0, 2.0, 1.0, 1.0, 0.0}};

	if(argc < 2) {
		printf("Type Input Image Name\t---> ");
		scanf("%s", inputImage);
	} else {
		strcpy(inputImage, argv[1]);
	}
	sprintf(outputImage, "log.jpg");

	jpeg_create_decompress(&cinfo);
	cinfo.err = jpeg_std_error(&jerr);
	if((fp = fopen(inputImage, "rb")) == NULL) {
		perror("fopen");
		exit(1);
	}

	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	JSAMPARRAY inImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * cinfo.output_height);
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

	out_width  = cinfo.image_width;
	out_height = cinfo.image_height;

	/* グレースケールへ変換 */
	JSAMPARRAY grayImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		grayImg[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width);
		for(int j = 0; j < out_width; j++) {
			grayImg[i][j] = 0.3 * inImg[i][j * 3 + 0] + 0.59 * inImg[i][j * 3 + 1] + 0.11 * inImg[i][j * 3 + 2];		//グレースケール化
		}
	}

	coutfo.image_width		= out_width;
	coutfo.image_height		= out_height;
	coutfo.input_components = 1;
	coutfo.in_color_space	= JCS_GRAYSCALE;
	jpeg_set_defaults(&coutfo);
	jpeg_set_quality(&coutfo, 75, TRUE);
	jpeg_start_compress(&coutfo, TRUE);
	int filterWidth	  = sizeof(laplacianFilter3[0]) / (2 * sizeof(double));
	JSAMPARRAY dogImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		dogImg[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width);
		for(int j = 0; j < out_width; j++) {
			float img_buf = 0.0;
			for(int l = -filterWidth; l <= filterWidth; l++) {
				for(int r = -filterWidth; r <= filterWidth; r++) {
					if((i + l < 0 || i + l >= out_height) || (j + r < 0 || j + r >= out_width)) {
						img_buf += (float)grayImg[i][j] * laplacianFilter3[l + filterWidth][r + filterWidth];
					} else {
						img_buf += (float)grayImg[i + l][j + r] * laplacianFilter3[l + filterWidth][r + filterWidth];
					}
				}
			}
			if(img_buf < 0) {
				dogImg[i][j] = 0;
			} else {
				dogImg[i][j] = (int)img_buf;
			}
		}
	}

	jpeg_write_scanlines(&coutfo, dogImg, out_height);
	jpeg_finish_compress(&coutfo);

	jpeg_destroy_compress(&coutfo);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	for(int i = 0; i < out_height; i++) {
		free(grayImg[i]);
		free(dogImg[i]);
	}
	free(grayImg);
	free(dogImg);
	fclose(gp);

	printf("Done\n");
	return 0;
}