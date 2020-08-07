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
	double max;
	double gaussianFilter3[3][3] = {
		{1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0},
		{2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0},
		{1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}};
	double gaussianFilter5[5][5] = {
		{1.0 / 256.0, 4.0 / 256.0, 6.0 / 256.0, 4.0 / 256.0, 1.0 / 256.0},
		{4.0 / 256.0, 16.0 / 256.0, 24.0 / 256.0, 16.0 / 256.0, 4.0 / 256.0},
		{6.0 / 256.0, 24.0 / 256.0, 36.0 / 256.0, 24.0 / 256.0, 6.0 / 256.0},
		{4.0 / 256.0, 16.0 / 256.0, 24.0 / 256.0, 16.0 / 256.0, 4.0 / 256.0},
		{1.0 / 256.0, 4.0 / 256.0, 6.0 / 256.0, 4.0 / 256.0, 1.0 / 256.0}};
	double gaussianFilter7[7][7] = {
		{1.0 / 4096.0, 6.0 / 4096.0, 15.0 / 4096.0, 20.0 / 4096.0, 15.0 / 4096.0, 6.0 / 4096.0, 1.0 / 4096},
		{6.0 / 4096.0, 36.0 / 4096.0, 90.0 / 4096.0, 120.0 / 4096.0, 90.0 / 4096.0, 36.0 / 4096.0, 6.0 / 4096},
		{15.0 / 4096.0, 90.0 / 4096.0, 225.0 / 4096.0, 300.0 / 4096.0, 225.0 / 4096.0, 90.0 / 4096.0, 15.0 / 4096},
		{20.0 / 4096.0, 120.0 / 4096.0, 300.0 / 4096.0, 400.0 / 4096.0, 300.0 / 4096.0, 120.0 / 4096.0, 20.0 / 4096},
		{15.0 / 4096.0, 90.0 / 4096.0, 225.0 / 4096.0, 300.0 / 4096.0, 225.0 / 4096.0, 90.0 / 4096.0, 15.0 / 4096},
		{6.0 / 4096.0, 36.0 / 4096.0, 90.0 / 4096.0, 120.0 / 4096.0, 90.0 / 4096.0, 36.0 / 4096.0, 6.0 / 4096},
		{1.0 / 4096.0, 6.0 / 4096.0, 15.0 / 4096.0, 20.0 / 4096.0, 15.0 / 4096.0, 6.0 / 4096.0, 1.0 / 4096}};
	double gaussianFilter9[9][9] = {
		{1.0 / 65536.0, 8.0 / 65536.0, 28.0 / 65536.0, 56.0 / 65536.0, 70.0 / 65536.0, 56.0 / 65536.0, 28.0 / 65536.0, 8.0 / 65536.0, 1.0 / 65536.0},
		{8.0 / 65536.0, 64.0 / 65536.0, 224.0 / 65536.0, 448.0 / 65536.0, 560.0 / 65536.0, 448.0 / 65536.0, 224.0 / 65536.0, 64.0 / 65536.0, 8.0 / 65536.0},
		{28.0 / 65536.0, 224.0 / 65536.0, 784.0 / 65536.0, 1568.0 / 65536.0, 1960.0 / 65536.0, 1568.0 / 65536.0, 784.0 / 65536.0, 224.0 / 65536.0, 28.0 / 65536.0},
		{56.0 / 65536.0, 448.0 / 65536.0, 1568.0 / 65536.0, 3136.0 / 65536.0, 3920.0 / 65536.0, 3136.0 / 65536.0, 1568.0 / 65536.0, 448.0 / 65536.0, 56.0 / 65536.0},
		{70.0 / 65536.0, 560.0 / 65536.0, 1960.0 / 65536.0, 3920.0 / 65536.0, 4900.0 / 65536.0, 3920.0 / 65536.0, 1960.0 / 65536.0, 560.0 / 65536.0, 70.0 / 65536.0},
		{56.0 / 65536.0, 448.0 / 65536.0, 1568.0 / 65536.0, 3136.0 / 65536.0, 3920.0 / 65536.0, 3136.0 / 65536.0, 1568.0 / 65536.0, 448.0 / 65536.0, 56.0 / 65536.0},
		{28.0 / 65536.0, 224.0 / 65536.0, 784.0 / 65536.0, 1568.0 / 65536.0, 1960.0 / 65536.0, 1568.0 / 65536.0, 784.0 / 65536.0, 224.0 / 65536.0, 28.0 / 65536.0},
		{8.0 / 65536.0, 64.0 / 65536.0, 224.0 / 65536.0, 448.0 / 65536.0, 560.0 / 65536.0, 448.0 / 65536.0, 224.0 / 65536.0, 64.0 / 65536.0, 8.0 / 65536.0},
		{1.0 / 65536.0, 8.0 / 65536.0, 28.0 / 65536.0, 56.0 / 65536.0, 70.0 / 65536.0, 56.0 / 65536.0, 28.0 / 65536.0, 8.0 / 65536.0, 1.0 / 65536.0}};

	if(argc < 2) {
		printf("Type Input Image Name\t---> ");
		scanf("%s", inputImage);
	} else {
		strcpy(inputImage, argv[1]);
	}
	sprintf(outputImage, "dog.jpg");

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
	double*** dogBuf = (double***)malloc(sizeof(double) * out_height);
	int filterWidth1 = sizeof(gaussianFilter5[0]) / (2 * sizeof(double));
	int filterWidth2 = sizeof(gaussianFilter7[0]) / (2 * sizeof(double));
	for(int i = 0; i < out_height; i++) {
		dogBuf[i] = (double**)malloc(sizeof(double) * out_width);
		for(int j = 0; j < out_width; j++) {
			dogBuf[i][j]	= (double*)malloc(sizeof(double) * 2);
			double img_buf1 = 0.0;
			double img_buf2 = 0.0;
			for(int l = -filterWidth1; l <= filterWidth1; l++) {
				for(int r = -filterWidth1; r <= filterWidth1; r++) {
					if((i + l < 0 || i + l >= out_height) || (j + r < 0 || j + r >= out_width)) {
						img_buf1 += (double)grayImg[i][j] * gaussianFilter5[l + filterWidth1][r + filterWidth1];
					} else {
						img_buf1 += (double)grayImg[i + l][j + r] * gaussianFilter5[l + filterWidth1][r + filterWidth1];
					}
				}
			}
			for(int l = -filterWidth2; l <= filterWidth2; l++) {
				for(int r = -filterWidth2; r <= filterWidth2; r++) {
					if((i + l < 0 || i + l >= out_height) || (j + r < 0 || j + r >= out_width)) {
						img_buf2 += (double)grayImg[i][j] * gaussianFilter7[l + filterWidth2][r + filterWidth2];
					} else {
						img_buf2 += (double)grayImg[i + l][j + r] * gaussianFilter7[l + filterWidth2][r + filterWidth2];
					}
				}
			}
			dogBuf[i][j][0] = img_buf1;
			dogBuf[i][j][1] = img_buf2;
			if(max < dogBuf[i][j][0] - dogBuf[i][j][1]) {
				max = dogBuf[i][j][0] - dogBuf[i][j][1];
			}
		}
	}

	JSAMPARRAY dogImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		dogImg[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width);
		for(int j = 0; j < out_width; j++) {
			if(dogBuf[i][j][0] - dogBuf[i][j][1] <= 0) {
				dogImg[i][j] = 0;
			} else {
				dogImg[i][j] = (int)((dogBuf[i][j][0] - dogBuf[i][j][1]) * 255.0 / max);
			}
			//printf("%d\n", dogImg[i][j]);
			free(dogBuf[i][j]);
		}
		free(dogBuf[i]);
	}
	free(dogBuf);

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