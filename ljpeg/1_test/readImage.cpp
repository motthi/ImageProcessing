#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <jpeglib.h>

int main(int argc, char** argv) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE* fp;
	char inputImage[255];

	/* コマンドライン引数の読み込み */
	if(argc == 1) {
		scanf("%s", inputImage);
	} else {
		strcpy(inputImage, argv[1]);
	}

	/* ファイル読み込み */
	jpeg_create_decompress(&cinfo);		   //オブジェクトの初期化
	cinfo.err = jpeg_std_error(&jerr);
	if((fp = fopen(inputImage, "rb")) == NULL) {
		perror("fopen");
		exit(1);
	}
	jpeg_stdio_src(&cinfo, fp);		   //読み込むファイルを指定

	/* ヘッダ情報の読み込み */
	jpeg_read_header(&cinfo, FALSE);		//ヘッダ情報の読み込み
	printf("%d\t%d\t%d\n", cinfo.image_width, cinfo.image_height, cinfo.num_components);

	/* データの読み込み */
	jpeg_start_decompress(&cinfo);														   //画像情報の取得
	JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * cinfo.output_height);		   //メモリ確保
	for(int i = 0; i < cinfo.output_height; ++i) {
		buffer[i] = (JSAMPROW)calloc(sizeof(JSAMPLE), cinfo.output_width * cinfo.output_components);
	}
	while(cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);		 //1列ずつ読み込む
	}

	/* 構造体破棄 */
	jpeg_finish_decompress(&cinfo);			//復元処理完了
	jpeg_destroy_decompress(&cinfo);		//オブジェクト破棄
	fclose(fp);

	/* データ出力 */
	for(int i = 0; i < cinfo.output_height; ++i) {
		printf("%d:", i);
		for(int j = 0; j < cinfo.output_width; ++j) {
			printf("[%d,%d,%d]", buffer[i][j * 3], buffer[i][j * 3 + 1], buffer[i][j * 3 + 2]);
		}
		printf("\n");
	}

	printf("Done\n");
	return 0;
}