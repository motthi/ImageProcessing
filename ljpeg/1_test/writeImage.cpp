#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

int main() {
	/* JPEGオブジェクト, エラーハンドラの確保 */
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);		  //エラーハンドラにデフォルト値を設定
	jpeg_create_compress(&cinfo);			  //JPEGオブジェクトの初期化

	/* 出力ファイルの設定 */
	char* filename = "output.jpg";
	FILE* fp	   = fopen(filename, "wb");
	if(fp == NULL) {
		fprintf(stderr, "cannot open %s\n", filename);
		exit(EXIT_FAILURE);
	}
	jpeg_stdio_dest(&cinfo, fp);		//出力先の指定

	/* 画像のパラメータの設定 */
	cinfo.image_width	   = 256;
	cinfo.image_height	   = 256;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;
	jpeg_set_defaults(&cinfo);				   //パラメータをデフォルトに設定
	jpeg_set_quality(&cinfo, 75, TRUE);		   //圧縮品質を設定

	/* 圧縮開始 */
	jpeg_start_compress(&cinfo, TRUE);		  //圧縮開始

	/* RGB値の設定 */
	JSAMPARRAY img = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * cinfo.image_height);
	for(int i = 0; i < cinfo.image_height; i++) {
		img[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * 3 * cinfo.image_width);
		for(int j = 0; j < cinfo.image_width; j++) {
			img[i][j * 3 + 0] = i;
			img[i][j * 3 + 1] = j;
			img[i][j * 3 + 2] = 127;
		}
	}
	jpeg_write_scanlines(&cinfo, img, cinfo.image_height);		  //書き込み
	jpeg_finish_compress(&cinfo);								  //圧縮完了

	/* オブジェクトの破棄，メモリの解放 */
	jpeg_destroy_compress(&cinfo);		  //オブジェクトの破棄
	for(int i = 0; i < cinfo.image_height; i++) {
		free(img[i]);
	}
	free(img);
	fclose(fp);
}