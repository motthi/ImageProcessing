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
	char filterName[255];
	float filter[3][3];
	float grayscale[3][3]			 = {{0.0, 0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}};
	float moveMeanFilter[3][3]		 = {{1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0}, {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0}, {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0}};
	float gaussianFilter[3][3]		 = {{1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}, {2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0}, {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}};
	float sobelFilter_side[3][3]	 = {{1.0, 2.0, 1.0}, {0.0, 0.0, 0.0}, {-1.0, -2.0, -1.0}};
	float sobelFilter_vertical[3][3] = {{1.0, 0.0, -1.0}, {2.0, 0.0, -2.0}, {1.0, 0.0, -1.0}};

	if(argc < 2) {
		printf("Type Input Image Name\t---> ");
		scanf("%s", inputImage);
	} else {
		strcpy(inputImage, argv[1]);
	}

	/* コマンドライン引数からフィルタを指定 */
	if(argc < 3) {		  //指定されていなければ打ち込んでもらう．
		printf("Type Filter Name\t---> ");
		scanf("%s", filterName);
	} else {
		strcpy(filterName, argv[2]);
	}

	/* 指定されたフィルタを配列に代入，出力ファイル名を指定 */
	if(strcmp(filterName, "sobel_vertical") == 0) {
		for(int i = 0; i <= 2; i++) {
			for(int j = 0; j <= 2; j++) {
				filter[i][j] = sobelFilter_vertical[i][j];
			}
		}
	} else if(strcmp(filterName, "sobel_side") == 0) {
		for(int i = 0; i <= 2; i++) {
			for(int j = 0; j <= 2; j++) {
				filter[i][j] = sobelFilter_side[i][j];
			}
		}
	} else if(strcmp(filterName, "moveMean") == 0) {
		for(int i = 0; i <= 2; i++) {
			for(int j = 0; j <= 2; j++) {
				filter[i][j] = moveMeanFilter[i][j];
			}
		}
	} else if(strcmp(filterName, "gaussian") == 0) {
		for(int i = 0; i <= 2; i++) {
			for(int j = 0; j <= 2; j++) {
				filter[i][j] = gaussianFilter[i][j];
			}
		}
	} else if(strcmp(filterName, "grayscale") == 0) {
		for(int i = 0; i <= 2; i++) {
			for(int j = 0; j <= 2; j++) {
				filter[i][j] = grayscale[i][j];
			}
		}
	} else {
		printf("\n\nError:No such filter\n");
		exit(1);
	}
	sprintf(outputImage, "%s.jpg", filterName);

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

	coutfo.image_width		= out_width;
	coutfo.image_height		= out_height;
	coutfo.input_components = 1;					//グレースケールはチャンネル数を1
	coutfo.in_color_space	= JCS_GRAYSCALE;		//グレースケールに設定
	jpeg_set_defaults(&coutfo);
	jpeg_set_quality(&coutfo, 75, TRUE);

	jpeg_start_compress(&coutfo, TRUE);

	/* グレースケールへ変換 */
	JSAMPARRAY grayImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		grayImg[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width);
		for(int j = 0; j < out_width; j++) {
			grayImg[i][j] = 0.3 * buffer[i][j * 3 + 0] + 0.59 * buffer[i][j * 3 + 1] + 0.11 * buffer[i][j * 3 + 2];		   //グレースケール化
		}
	}

	JSAMPARRAY sobelImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		sobelImg[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width);
		for(int j = 0; j < out_width; j++) {
			float img_buf = 0.0;
			for(int l = -1; l <= 1; l++) {
				for(int r = -1; r <= 1; r++) {
					if((i + l < 0 || i + l >= out_height) || (j + r < 0 || j + r >= out_width)) {
						img_buf += (float)grayImg[i][j] * filter[l + 1][r + 1];
					} else {
						img_buf += (float)grayImg[i + l][j + r] * filter[l + 1][r + 1];
					}
				}
			}
			if(img_buf < 0) {
				sobelImg[i][j] = 0;
			} else {
				sobelImg[i][j] = (int)img_buf;
			}
		}
	}

	jpeg_write_scanlines(&coutfo, sobelImg, out_height);
	jpeg_finish_compress(&coutfo);

	jpeg_destroy_compress(&coutfo);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	for(int i = 0; i < out_height; i++) {
		free(grayImg[i]);
	}
	free(grayImg);
	for(int i = 0; i < out_height; i++) {
		free(sobelImg[i]);
	}
	free(sobelImg);
	fclose(gp);

	printf("Done\n");
	return 0;
}