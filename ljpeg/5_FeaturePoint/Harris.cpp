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
	double sobelFilter_side[3][3]	  = {{1.0, 2.0, 1.0}, {0.0, 0.0, 0.0}, {-1.0, -2.0, -1.0}};
	double sobelFilter_vertical[3][3] = {{1.0, 0.0, -1.0}, {2.0, 0.0, -2.0}, {1.0, 0.0, -1.0}};
	double** ix;		//画像の勾配（ソーベルフィルタ水平の結果）
	double** iy;		//画像の勾配（ソーベルフィルタ垂直の結果）
	double** R;
	double k   = 0.15;
	double max = 0.0;

	if(argc < 2) {
		printf("Type Input Image Name\t---> ");
		scanf("%s", inputImage);
	} else {
		strcpy(inputImage, argv[1]);
	}
	sprintf(outputImage, "harris.jpg");

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
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
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(fp);

	coutfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&coutfo);
	if((gp = fopen(outputImage, "wb")) == NULL) {
		perror("fopen");
		exit(1);
	}

	out_width  = cinfo.image_width;
	out_height = cinfo.image_height;

	/* グレースケールへ変換 */
	JSAMPARRAY grayImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		grayImg[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width);
		for(int j = 0; j < out_width; j++) {
			if(cinfo.output_components == 3) {
				grayImg[i][j] = 0.3 * inImg[i][j * 3 + 0] + 0.59 * inImg[i][j * 3 + 1] + 0.11 * inImg[i][j * 3 + 2];		//グレースケール化
			} else if(cinfo.output_components == 1) {
				grayImg[i][j] = inImg[i][j];
			}
		}
	}

	ix = (double**)malloc(sizeof(double) * out_height);
	iy = (double**)malloc(sizeof(double) * out_height);
	for(int i = 0; i < out_height; i++) {
		ix[i] = (double*)malloc(sizeof(double) * out_width);
		iy[i] = (double*)malloc(sizeof(double) * out_width);
		for(int j = 0; j < out_width; j++) {
			ix[i][j] = 0.0;
			iy[i][j] = 0.0;
			for(int l = -1; l <= 1; l++) {
				for(int r = -1; r <= 1; r++) {
					if((i + l < 0 || i + l >= out_height) || (j + r < 0 || j + r >= out_width)) {
						ix[i][j] += grayImg[i][j] * sobelFilter_side[l + 1][r + 1];
						iy[i][j] += grayImg[i][j] * sobelFilter_vertical[l + 1][r + 1];
					} else {
						ix[i][j] += grayImg[i + l][j + r] * sobelFilter_side[l + 1][r + 1];
						iy[i][j] += grayImg[i + l][j + r] * sobelFilter_vertical[l + 1][r + 1];
					}
				}
			}
		}
	}

	R = (double**)malloc(sizeof(double) * out_height);
	for(int i = 0; i < out_height; i++) {
		R[i] = (double*)malloc(sizeof(double) * out_width);
		for(int j = 0; j < out_width; j++) {
			double m_00 = 0.0;
			double m_01 = 0.0;
			double m_11 = 0.0;
			for(int l = -1; l <= 1; l++) {
				for(int r = -1; r <= 1; r++) {
					if((i + l < 0 || i + l >= out_height) || (j + r < 0 || j + r >= out_width)) {
						R[i][j] = 0.0;
						continue;
					} else {
						m_00 += ix[i + l][j + r] * ix[i + l][j + r];
						m_01 += ix[i + l][j + r] * iy[i + l][j + r];
						m_11 += iy[i + l][j + r] * iy[i + l][j + r];
					}
				}
			}
			R[i][j] = (m_00 * m_11 - m_01 * m_01) - k * (m_00 + m_11) * (m_00 + m_11);
			if(max < R[i][j]) {
				max = R[i][j];
			}
		}
	}
	printf("%f\n", max);

	jpeg_stdio_dest(&coutfo, gp);
	coutfo.image_width		= out_width;
	coutfo.image_height		= out_height;
	coutfo.input_components = 3;
	coutfo.in_color_space	= JCS_RGB;
	jpeg_set_defaults(&coutfo);
	jpeg_set_quality(&coutfo, 75, TRUE);
	jpeg_start_compress(&coutfo, TRUE);
	JSAMPARRAY harrisImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		free(ix[i]);
		free(iy[i]);
		harrisImg[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width * 3);
		for(int j = 0; j < out_width; j++) {
			if(cinfo.output_components == 1) {
				harrisImg[i][j * 3 + 0] = inImg[i][j];
				harrisImg[i][j * 3 + 1] = inImg[i][j];
				harrisImg[i][j * 3 + 2] = inImg[i][j];
			} else if(cinfo.output_components == 3) {
				harrisImg[i][j * 3 + 0] = inImg[i][j * 3 + 0];
				harrisImg[i][j * 3 + 1] = inImg[i][j * 3 + 1];
				harrisImg[i][j * 3 + 2] = inImg[i][j * 3 + 2];
			}
			R[i][j] = R[i][j] * 255.0 / max;
			if(R[i][j] <= 0) {
				/* Edge */
				//harrisImg[i][j * 3 + 0] = 255;
				//harrisImg[i][j * 3 + 1] = 0;
				//harrisImg[i][j * 3 + 2] = 0;
			} else if(abs(R[i][j]) < 1) {
				/* Flat */
				//harrisImg[i][j * 3 + 0] = 255;
				//harrisImg[i][j * 3 + 1] = 0;
				//harrisImg[i][j * 3 + 2] = 0;
			} else {
				/* Corner */
				harrisImg[i][j * 3 + 0] = 255;
				harrisImg[i][j * 3 + 1] = 0;
				harrisImg[i][j * 3 + 2] = 0;
			}
			/* if(R[i][j] > 1) {
				printf("%d\t%d\t%f\n", i, j, R[i][j]);
			} */
		}
		free(R[i]);
	}
	free(ix);
	free(iy);
	free(R);

	jpeg_write_scanlines(&coutfo, harrisImg, out_height);
	jpeg_finish_compress(&coutfo);

	jpeg_destroy_compress(&coutfo);
	for(int i = 0; i < out_height; i++) {
		free(grayImg[i]);
		free(harrisImg[i]);
	}
	free(grayImg);
	free(harrisImg);
	fclose(gp);

	printf("Done\n");
	return 0;
}