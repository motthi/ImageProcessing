#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <jpeglib.h>
#include <math.h>
#include <iostream>

#define PI 4.0 * atan(1.0)		  //円周率
#define HIGH 255				  // ２値画像のハイ・レベル
#define LOW 0					  // ２値画像のロー・レベル
#define OPT 1					  //　OPT = 1　光学的ＤＦＴ（直流分が中央），OPT = 0　通常のＤＦＴ（直流分が左端）

int fft2(double** a_rl, double** a_im, int inv, int xsize, int ysize);
void fft1core(double* a_rl, double* a_im, int length, int ex, double* sin_tbl, double* cos_tbl, double* buf);
void cstb(int length, int inv, double* sin_tbl, double* cos_tbl);
void birv(double* a, int length, int ex, double* b);
void rvmtx(double** a, double** b, int xsize, int ysize);

int main(int argc, char** argv) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_compress_struct coutfo;
	struct jpeg_error_mgr jerr;
	int out_height, out_width;
	FILE* fp;
	FILE* gp;
	char inputImage[255];
	char outputImage[255] = "fft.jpg";
	double** fft_c;
	double** fft_s;
	double max;

	if(argc < 2) {
		printf("Type Input Image Name\t---> ");
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
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
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
			grayImg[i][j] = 0.3 * buffer[i][j * 3 + 0] + 0.59 * buffer[i][j * 3 + 1] + 0.11 * buffer[i][j * 3 + 2];		   //グレースケール化
		}
	}

	/* FFT用配列の初期化 */
	int xexp = 0, yexp = 0;
	while(cinfo.output_width > pow(2, xexp)) {
		xexp++;
	}
	while(cinfo.output_height > pow(2, yexp)) {
		yexp++;
	}
	fft_c = (double**)malloc(sizeof(double) * pow(2, yexp));
	fft_s = (double**)malloc(sizeof(double) * pow(2, yexp));
	for(int i = 0; i < pow(2, yexp); i++) {
		fft_c[i] = (double*)malloc(sizeof(double) * pow(2, xexp));
		fft_s[i] = (double*)malloc(sizeof(double) * pow(2, xexp));
		//printf("%d\n", i);
		for(int j = 0; j < pow(2, xexp); j++) {
			fft_c[i][j] = 0.0;
			fft_s[i][j] = 0.0;
			if(j >= out_width) {
				if(i >= out_height) {
					fft_c[i][j] = (double)grayImg[out_height - 1][out_width - 1];
				} else {
					fft_c[i][j] = (double)grayImg[i][out_width - 1];
				}
			} else {
				if(i >= out_height) {
					fft_c[i][j] = (double)grayImg[out_height - 1][j];
				} else {
					fft_c[i][j] = (double)grayImg[i][j];
				}
			}
		}
	}

	/* FFT */
	if(fft2(fft_c, fft_s, 1, pow(2, xexp), pow(2, yexp)) == -1) {
		return -1;
	}

	/* 周波数フィルタ */
	coutfo.image_width		= pow(2, xexp);
	coutfo.image_height		= pow(2, yexp);
	coutfo.input_components = 1;					//グレースケールはチャンネル数を1
	coutfo.in_color_space	= JCS_GRAYSCALE;		//グレースケールに設定
	//jpeg_set_defaults(&coutfo);
	//jpeg_set_quality(&coutfo, 75, TRUE);
	//jpeg_start_compress(&coutfo, TRUE);
	int filter_freq = 300;
	for(int i = 0; i < pow(2, yexp); i++) {
		for(int j = 0; j < pow(2, xexp); j++) {
			if(abs(i - pow(2, yexp - 1)) < filter_freq && abs(j - pow(2, xexp - 1)) < filter_freq) {
				//fft_c[i][j] = 0;
				//fft_s[i][j] = 0;
			} else {
				fft_c[i][j] = 0;
				fft_s[i][j] = 0;
			}
		}
	}

	/* FFT結果の可視化 */
	/* max					= 0.0;
	double** fftImg_buf = (double**)malloc(sizeof(double) * pow(2, yexp));
	for(int i = 0; i < pow(2, yexp); i++) {
		fftImg_buf[i] = (double*)malloc(sizeof(double) * pow(2, xexp));
		for(int j = 0; j < pow(2, xexp); j++) {
			fftImg_buf[i][j] = 0.0;
			fftImg_buf[i][j] = fft_c[i][j] * fft_c[i][j] + fft_s[i][j] * fft_s[i][j];
			if(fftImg_buf[i][j] != 0.0) {
				fftImg_buf[i][j] = log(fftImg_buf[i][j]) / 2.0;
			} else {
				fftImg_buf[i][j] = 0;
			}
			if(max < fftImg_buf[i][j]) {
				max = fftImg_buf[i][j];
			}
			if(fftImg_buf[i][j] < 0) {
				fftImg_buf[i][j] = 0;
			}
		}
	} */

	/* 画像の保存 */
	/* JSAMPARRAY fftImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * pow(2, yexp));
	for(int i = 0; i < pow(2, yexp); i++) {
		fftImg[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * pow(2, xexp));
		for(int j = 0; j < pow(2, xexp); j++) {
			fftImg[i][j] = (int)(fftImg_buf[i][j] * 255.0 / max);
			if(fftImg[i][j] > 255) {
				fftImg[i][j] = 255;
			} else if(fftImg[i][j] < 0) {
				fftImg[i][j] = 0;
			}
		}
	} */

	/* DFFT */
	if(fft2(fft_c, fft_s, -1, pow(2, xexp), pow(2, yexp)) == -1) {
		printf("\n\nError : malloc");
		return -1;
	}

	/* フィルタ後の画像の可視化 */
	coutfo.image_width		= out_width;
	coutfo.image_height		= out_height;
	coutfo.input_components = 1;					//グレースケールはチャンネル数を1
	coutfo.in_color_space	= JCS_GRAYSCALE;		//グレースケールに設定 max					= 0.0;
	jpeg_set_defaults(&coutfo);
	jpeg_set_quality(&coutfo, 75, TRUE);
	jpeg_start_compress(&coutfo, TRUE);
	double** fftImg_buf = (double**)malloc(sizeof(double) * out_height);
	for(int i = 0; i < out_height; i++) {
		fftImg_buf[i] = (double*)malloc(sizeof(double) * out_width);
		for(int j = 0; j < out_width; j++) {
			fftImg_buf[i][j] = 0.0;
			fftImg_buf[i][j] = sqrt(fft_c[i][j] * fft_c[i][j] + fft_s[i][j] * fft_s[i][j]);
			if(max < fftImg_buf[i][j]) {
				max = fftImg_buf[i][j];
			}
			if(fftImg_buf[i][j] < 0) {
				fftImg_buf[i][j] = 0;
			}
		}
	}

	/* 画像の保存 */
	JSAMPARRAY fftImg = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * out_height);
	for(int i = 0; i < out_height; i++) {
		fftImg[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * out_width);
		for(int j = 0; j < out_width; j++) {
			fftImg[i][j] = (int)(fftImg_buf[i][j] * 255.0 / max);
			if(fftImg[i][j] > 255) {
				fftImg[i][j] = 255;
			} else if(fftImg[i][j] < 0) {
				fftImg[i][j] = 0;
			}
		}
	}

	//jpeg_write_scanlines(&coutfo, fftImg, pow(2, yexp));
	jpeg_write_scanlines(&coutfo, fftImg, out_height);
	jpeg_finish_compress(&coutfo);

	jpeg_destroy_compress(&coutfo);
	for(int i = 0; i < out_height; i++) {
		free(grayImg[i]);
		free(fftImg_buf[i]);
		free(fftImg[i]);
	}
	free(grayImg);
	free(fftImg);
	free(fftImg_buf);
	fclose(gp);

	printf("Done\n");
	return 0;
}

/*--- fft2 --- ２次元ＦＦＴの実行 ---------------------------------------------
		（xsize，ysizeが２のべき乗の場合に限る）
	a_rl:	データ実数部（入出力兼用）
	a_im:	データ虚数部（入出力兼用）
	inv:	1: ＤＦＴ，-1: 逆ＤＦＴ
-----------------------------------------------------------------------------*/
int fft2(double** a_rl, double** a_im, int inv, int xsize, int ysize) {
	double** b_rl;	  /* データ転置作業用配列（実数部）*/
	double** b_im;	  /* データ転置作業用配列（虚数部）*/
	double* hsin_tbl; /* 水平用SIN計算用テーブル */
	double* hcos_tbl; /* 水平用COS計算用テーブル */
	double* vsin_tbl; /* 垂直用SIN計算用テーブル */
	double* vcos_tbl; /* 垂直用COS計算用テーブル */
	double* buf_x;	  /* 作業用バッファ（水平方向）	*/
	double* buf_y;	  /* 作業用バッファ（垂直方向）	*/
	int xexp = 0;
	int yexp = 0;

	int mod = xsize;
	while(mod != 1) {
		if(mod % 2 == 1) {
			printf("\n\nError : Photo size is not2^n\n");
			return -1;
		}
		mod /= 2;
		xexp++;
	}
	mod = ysize;
	while(mod != 1) {
		if(mod % 2 == 1) {
			printf("\n\nError : Photo size is not2^n\n");
			return -1;
		}
		mod /= 2;
		yexp++;
	}

	hsin_tbl = (double*)calloc((size_t)xsize, sizeof(double));
	hcos_tbl = (double*)calloc((size_t)xsize, sizeof(double));
	vsin_tbl = (double*)calloc((size_t)ysize, sizeof(double));
	vcos_tbl = (double*)calloc((size_t)ysize, sizeof(double));
	buf_x	 = (double*)malloc((size_t)xsize * sizeof(double));
	buf_y	 = (double*)malloc((size_t)ysize * sizeof(double));
	b_rl	 = (double**)malloc(sizeof(double) * xsize);
	b_im	 = (double**)malloc(sizeof(double) * xsize);
	for(int i = 0; i < xsize; i++) {
		b_rl[i] = (double*)malloc(sizeof(double) * ysize);
		b_im[i] = (double*)malloc(sizeof(double) * ysize);
		if(b_rl[i] == NULL || b_im[i] == NULL) {
			perror("malloc");
			return -1;
		}
	}
	if((b_rl == NULL) || (b_im == NULL) || (hsin_tbl == NULL) || (hcos_tbl == NULL) || (vsin_tbl == NULL) || (vcos_tbl == NULL) || (buf_x == NULL) || (buf_y == NULL)) {
		return -1;
	}
	cstb(xsize, inv, hsin_tbl, hcos_tbl); /* 水平用SIN,COSテーブル作成	*/
	cstb(ysize, inv, vsin_tbl, vcos_tbl); /* 垂直用SIN,COSテーブル作成	*/

	/* 水平方向のＦＦＴ */
	for(int i = 0; i < ysize; i++) {
		fft1core(a_rl[i], a_im[i], xsize, xexp, hsin_tbl, hcos_tbl, buf_x);
	}

	/* ２次元データの転置 */
	rvmtx(a_rl, b_rl, xsize, ysize);
	rvmtx(a_im, b_im, xsize, ysize);

	/* 垂直方向のＦＦＴ */
	for(int i = 0; i < xsize; i++) {
		fft1core(b_rl[i], b_im[i], ysize, yexp, vsin_tbl, vcos_tbl, buf_y);
	}

	/* ２次元データの転置 */
	rvmtx(b_rl, a_rl, ysize, xsize);
	rvmtx(b_im, a_im, ysize, xsize);

	/* メモリ領域の解法 */
	for(int i = 0; i < ysize; i++) {
		free(b_rl[i]);
		free(b_im[i]);
	}
	free(b_rl);
	free(b_im);
	free(hsin_tbl);
	free(hcos_tbl);
	free(vsin_tbl);
	free(vcos_tbl);

	return 0;
}

/*--- fft1core --- １次元ＦＦＴの計算の核になる部分 ---------------------------
	a_rl:	データ実数部（入出力兼用）
	a_im:	データ虚数部（入出力兼用）
	ex:		データ個数を２のべき乗で与える(データ個数 = 2のex乗個）
	sin_tbl:	SIN計算用テーブル
	cos_tbl:	COS計算用テーブル
-----------------------------------------------------------------------------*/
void fft1core(double* a_rl, double* a_im, int length, int ex, double* sin_tbl, double* cos_tbl, double* buf) {
	int w, j1, j2;
	int numb, lenb, timb;
	double xr, xi, yr, yi, nrml;

	if(OPT == 1) {
		for(int i = 1; i < length; i += 2) {
			a_rl[i] = -a_rl[i];
			a_im[i] = -a_im[i];
		}
	}
	numb = 1;
	lenb = length;
	for(int i = 0; i < ex; i++) {
		lenb /= 2;
		timb = 0;
		for(int j = 0; j < numb; j++) {
			w = 0;
			for(int k = 0; k < lenb; k++) {
				j1		 = timb + k;
				j2		 = j1 + lenb;
				xr		 = a_rl[j1];
				xi		 = a_im[j1];
				yr		 = a_rl[j2];
				yi		 = a_im[j2];
				a_rl[j1] = xr + yr;
				a_im[j1] = xi + yi;
				xr		 = xr - yr;
				xi		 = xi - yi;
				a_rl[j2] = xr * cos_tbl[w] - xi * sin_tbl[w];
				a_im[j2] = xr * sin_tbl[w] + xi * cos_tbl[w];
				w += numb;
			}
			timb += (2 * lenb);
		}
		numb *= 2;
	}
	birv(a_rl, length, ex, buf); /*　実数データの並べ換え　*/
	birv(a_im, length, ex, buf); /*　虚数データの並べ換え　*/
	if(OPT == 1) {
		for(int i = 1; i < length; i += 2) {
			a_rl[i] = -a_rl[i];
			a_im[i] = -a_im[i];
		}
	}
	nrml = (double)(1.0 / sqrt((double)length));
	for(int i = 0; i < length; i++) {
		a_rl[i] *= nrml;
		a_im[i] *= nrml;
	}
}

/*--- cstb --- SIN,COSテーブル作成 --------------------------------------------
	length:		データ個数
	inv:		1: ＤＦＴ, -1: 逆ＤＦＴ
	sin_tbl:	SIN計算用テーブル
	cos_tbl:	COS計算用テーブル
-----------------------------------------------------------------------------*/
void cstb(int length, int inv, double* sin_tbl, double* cos_tbl) {
	double xx, arg;

	xx = (double)(((-PI) * 2.0) / (double)length);
	if(inv < 0)
		xx = -xx;
	for(int i = 0; i < length; i++) {
		arg		   = (double)i * xx;
		sin_tbl[i] = (double)sin(arg);
		cos_tbl[i] = (double)cos(arg);
	}
}

/*--- birv --- データの並べ換え -----------------------------------------------
	a:		データの配列
	length:	データ個数
	ex:		データ個数を２のべき乗で与える(length = 2のex乗個）
	b:		作業用バッファ
-----------------------------------------------------------------------------*/
void birv(double* a, int length, int ex, double* b) {
	int ii, k, bit;

	for(int i = 0; i < length; i++) {
		for(k = 0, ii = i, bit = 0;; bit <<= 1, ii >>= 1) {
			bit = (ii & 1) | bit;
			if(++k == ex)
				break;
		}
		b[i] = a[bit];
	}
	for(int i = 0; i < length; i++) {
		a[i] = b[i];
	}
}

/*--- rvmtx1 --- ２次元データの転置 -------------------------------------------
	a:		２次元入力データ
	b:		２次元出力データ
	xsize:	水平データ個数
	ysize:	垂直データ個数
-----------------------------------------------------------------------------*/
void rvmtx(double** a, double** b, int xsize, int ysize) {
	for(int i = 0; i < ysize; i++) {
		for(int j = 0; j < xsize; j++) {
			//printf("\t%d\t%d\n", i, j);
			b[j][i] = a[i][j];
		}
	}
}