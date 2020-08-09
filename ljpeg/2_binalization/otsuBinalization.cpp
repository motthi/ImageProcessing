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
	int threshold		  = 0;

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

	/* 
	 * 大津の2値化，閾値を設定する
	 * 0~255すべての閾値で分離度を計算し，最大のものを取り出す
	 */
	float sb = 0.0;						  //クラス間分散
	for(int i = 0; i < 256; i++) {		  //0 ~ 255の閾値すべてでクラス間分散を計算
		float p0	 = 0;				  //"0"となる個数
		float p1	 = 0;				  //"1"となる個数
		float value0 = 0;				  //"0"となるピクセル値の総和
		float value1 = 0;				  //"1"となるピクセル値の総和
		for(int j = 0; j < out_height; j++) {
			for(int k = 0; k < out_width; k++) {
				int gray = (int)(0.3 * buffer[j][k * 3 + 0] + 0.59 * buffer[j][k * 3 + 1] + 0.11 * buffer[j][k * 3 + 2]);
				if(gray < i) {
					p0++;
					value0 += gray;
				} else {
					p1++;
					value1 += gray;
				}
			}
		}
		if(p0 == 0 || p1 == 0) {		//"0"または"1"となる個数が0ならば計算しない
			continue;
		}
		float mean0 = value0 / p0;
		float mean1 = value1 / p1;
		if(sb < p0 * p1 * (mean0 - mean1) * (mean0 - mean1)) {		  //より高いクラス間分散が得られれば更新
			sb		  = p0 * p1 * (mean0 - mean1) * (mean0 - mean1);
			threshold = i;
		}
		//printf("%d\t%f\t%f\t%f\t%f\t%f\n", i, p0, p1, mean0, mean1, sb);
	}
	printf("Threshold : %d\n", threshold);

	/* 得られた閾値で2値化 */
	JSAMPARRAY img = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		img[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width);
		for(int j = 0; j < out_width; j++) {
			if(0.3 * buffer[i][j * 3 + 0] + 0.59 * buffer[i][j * 3 + 1] + 0.11 * buffer[i][j * 3 + 2] < threshold) {
				img[i][j] = 0;
			} else {
				img[i][j] = 255;
			}
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