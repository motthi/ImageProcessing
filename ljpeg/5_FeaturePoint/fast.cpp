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

	if(argc < 2) {
		printf("Type Input Image Name\t---> ");
		scanf("%s", inputImage);
	} else {
		strcpy(inputImage, argv[1]);
	}
	sprintf(outputImage, "fast.jpg");

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
			grayImg[i][j] = 0.3 * inImg[i][j * 3 + 0] + 0.59 * inImg[i][j * 3 + 1] + 0.11 * inImg[i][j * 3 + 2];		//グレースケール化
		}
	}

	jpeg_stdio_dest(&coutfo, gp);
	coutfo.image_width		= out_width;
	coutfo.image_height		= out_height;
	coutfo.input_components = 3;
	coutfo.in_color_space	= JCS_RGB;
	jpeg_set_defaults(&coutfo);
	jpeg_set_quality(&coutfo, 75, TRUE);
	jpeg_start_compress(&coutfo, TRUE);
	int t			   = 10;
	JSAMPARRAY fastImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		fastImg[i] = (JSAMPROW)calloc(sizeof(JSAMPLE), out_width * 3);
		if(i < 4 || i > out_height - 4) {
			continue;
		}
		for(int j = 4; j < out_width - 4; j++) {
			fastImg[i][j * 3 + 0] = inImg[i][j * 3 + 0];
			fastImg[i][j * 3 + 1] = inImg[i][j * 3 + 1];
			fastImg[i][j * 3 + 2] = inImg[i][j * 3 + 2];
			if(grayImg[i][j] > grayImg[i][j - 3] + t) {
				if(grayImg[i][j] < grayImg[i + 1][j - 3] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i + 2][j - 2] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i + 3][j - 1] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i + 3][j] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i + 3][j + 1] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i + 2][j + 2] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i + 1][j + 3] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i][j + 3] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i - 1][j + 3] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i - 2][j + 2] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i - 3][j + 1] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i - 3][j] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i - 3][j - 1] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i - 2][j - 2] + t) {
					continue;
				}
				if(grayImg[i][j] < grayImg[i - 1][j - 3] + t) {
					continue;
				}
			} else if(grayImg[i][j] < grayImg[i][j - 3] - t) {
				if(grayImg[i][j] > grayImg[i + 1][j - 3] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i + 2][j - 2] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i + 3][j - 1] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i + 3][j] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i + 3][j + 1] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i + 2][j + 2] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i + 1][j + 3] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i][j + 3] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i - 1][j + 3] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i - 2][j + 2] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i - 3][j + 1] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i - 3][j] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i - 3][j - 1] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i - 2][j - 2] - t) {
					continue;
				}
				if(grayImg[i][j] > grayImg[i - 1][j - 3] - t) {
					continue;
				}
			} else {
				continue;
			}
			fastImg[i][j * 3 + 1] = 255;
			fastImg[i][j * 3 + 1] = 255;
			fastImg[i][j * 3 + 2] = 255;
		}
	}
	jpeg_write_scanlines(&coutfo, fastImg, out_height);
	jpeg_finish_compress(&coutfo);

	jpeg_destroy_compress(&coutfo);
	for(int i = 0; i < out_height; i++) {
		free(grayImg[i]);
		free(fastImg[i]);
	}
	free(grayImg);
	free(fastImg);
	fclose(gp);

	printf("Done\n");
	return 0;
}
