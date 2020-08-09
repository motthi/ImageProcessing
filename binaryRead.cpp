#include <string>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <fstream>
#include <vector>

std::string getBinary(char sin, int num = 0);

class jpegImage {
  public:
	double width;			   //ピクセル幅
	double height;			   //ピクセル高
	double component;		   //コンポーネント数
	int bits;				   //
	long long int size;		   //ファイルサイズ
	long int ss;			   //量子か係数開始番号
	long int se;			   //量子化係数終了番号
	char* photoData;		   //イメージ画像
	int imageStart;			   //イメージデータが始まるバイト

	std::string inputFileName;				  //ファイル名
	std::vector<std::vector<int>> sof;		  //SOF
	std::vector<std::vector<int>> sos;		  //スキャンヘッダ
	std::vector<std::vector<int>> dqt;		  //量子化テーブル定義

	jpegImage() {
		return;
	}

	~jpegImage() {
		delete photoData;
	}

	int readImageFile() {
		std::ifstream ifs(inputFileName, std::ios::binary);
		ifs.seekg(0, std::ios::end);
		size = ifs.tellg();
		ifs.seekg(0);
		photoData = new char[size];
		ifs.read(photoData, size);
		return 1;
	}

	void getHeader() {
		for(int i = 1; i < size + 1; i++) {
			std::string sbuf;
			sbuf = getBinary(photoData[i - 1]);
			if(sbuf == "ff") {
				std::string sbuf_next = getBinary(photoData[i]);
				if(sbuf_next == "c0" || sbuf_next == "c2") {		//SOF
					std::ostringstream ss_seg, ss_b, ss_h, ss_w, ss_c;
					long long int seg_length = 0;
					ss_seg << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 1]) << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 2]);
					ss_b << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 3]);
					ss_h << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 4]) << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 5]);
					ss_w << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 6]) << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 7]);
					ss_c << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 8]);
					bits	   = std::stoi(ss_b.str(), nullptr, 16);
					height	   = std::stoi(ss_h.str(), nullptr, 16);
					width	   = std::stoi(ss_w.str(), nullptr, 16);
					component  = std::stoi(ss_c.str(), nullptr, 16);
					seg_length = std::stoi(ss_seg.str(), nullptr, 16);
					for(int j = 0; j * 3 + 3 < seg_length - 6; j++) {
						sof.resize(j + 1);
						std::ostringstream ss_cn, ss_hn, ss_vn, ss_tqn;
						ss_cn << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 9 + j * 3]);
						ss_hn << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 9 + j * 3 + 1], 1);
						ss_vn << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 9 + j * 3 + 1], 2);
						ss_tqn << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 9 + j * 3 + 2]);
						sof[j].resize(4);
						sof[j][0] = std::stoi(ss_cn.str(), nullptr, 16);
						sof[j][1] = std::stoi(ss_hn.str(), nullptr, 16);
						sof[j][2] = std::stoi(ss_vn.str(), nullptr, 16);
						sof[j][3] = std::stoi(ss_tqn.str(), nullptr, 16);
					}
				} else if(sbuf_next == "db") {
					std::ostringstream ss_seg, ss_pqn, ss_tqn;
					long long int seg_length = 0;
					ss_seg << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 1]) << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 2]);
					seg_length = std::stoi(ss_seg.str(), nullptr, 16);
					for(int j = 3; j < seg_length; j++) {
						int k = dqt.size() + 1;
						dqt.resize(k);
						dqt[k - 1].resize(66);
						ss_pqn << std::setw(2) << std::setfill('0') << getBinary(photoData[i + j], 1);
						ss_tqn << std::setw(2) << std::setfill('0') << getBinary(photoData[i + j], 2);
						dqt[k - 1][0] = std::stoi(ss_pqn.str(), nullptr, 16);
						dqt[k - 1][1] = std::stoi(ss_tqn.str(), nullptr, 16);
						if(dqt[k - 1][0] == 0) {
							for(int q = 0; q < 64; q++) {
								std::ostringstream ss_q;
								ss_q << std::setw(2) << std::setfill('0') << getBinary(photoData[i + j + q + 2]);
								dqt[k - 1][q + 2] = std::stoi(ss_q.str(), nullptr, 16);
							}
							j += 64;
						} else if(dqt[k - 1][0] == 1) {
							for(int q = 0; q < 128; q++) {
								std::ostringstream ss_q;
								ss_q << std::setw(2) << std::setfill('0') << getBinary(photoData[i + j + q + 2]);
								dqt[k - 1][q + 2] = std::stoi(ss_q.str(), nullptr, 16);
							}
							j += 128;
						}
					}
				} else if(sbuf_next == "da") {
					std::ostringstream ss_seg, ss_ns, ss_ss, ss_se, ss_ah, ss_ai;
					long int seg_length = 0;
					long int ns			= 0;
					ss_seg << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 1]) << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 2]);
					ss_ns << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 3]);
					seg_length = std::stoi(ss_seg.str(), nullptr, 16);
					ns		   = std::stoi(ss_ns.str(), nullptr, 16);
					sos.resize(ns);
					for(int j = 0; j < ns; j++) {
						std::ostringstream ss_cs, ss_td, ss_ta;
						ss_cs << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 4 + j * 2]);
						ss_td << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 5 + j * 2], 1);
						ss_ta << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 5 + j * 2], 2);
						sos[j].resize(3);
						sos[j][0] = std::stoi(ss_cs.str(), nullptr, 16);
						sos[j][1] = std::stoi(ss_td.str(), nullptr, 16);
						sos[j][2] = std::stoi(ss_ta.str(), nullptr, 16);
					}
					ss_ss << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 4 + ns * 2]);
					ss_se << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 4 + ns * 2 + 1]);
					ss_ah << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 4 + ns * 2 + 2], 1);
					ss_ai << std::setw(2) << std::setfill('0') << getBinary(photoData[i + 4 + ns * 2 + 2], 2);

					ss = std::stoi(ss_ss.str(), nullptr, 16);
					se = std::stoi(ss_se.str(), nullptr, 16);
					//ah = std::stoi(ss_ah.str(), nullptr, 16);
					//ai = std::stoi(ss_ai.str(), nullptr, 16);
					imageStart = seg_length + i + 2;
				}
			}
		}
		return;
	}

	void getImageData() {
		printf("%lld\t%d\n", size, imageStart);
		int num = 0;
		for(int i = imageStart; i < size - 1; i++) {
			std::string sbuf;
			sbuf = getBinary(photoData[i - 1]);
			if(sbuf == "ff") {
				std::string sbuf_next = getBinary(photoData[i]);
				if(sbuf_next == "0") {
					i++;
				}
			}
			if(i % 32 == 0) {
				printf(" !!!\n");
			}
			std::cout << sbuf << " ";
			num++;
		}
		printf("\n");
		printf("num : %d\n", num);
		return;
	}

	void showAllData() {
		std::cout << "\nsize = " << size << "\n\n";		   //サイズを出力する
		for(int i = 1; i < size + 1; i++) {
			std::string sbuf;
			sbuf = getBinary(photoData[i - 1]);
			if(sbuf == "ff") {
				std::stringstream s_next;
				s_next << std::hex << +photoData[i];
				std::string test_next = s_next.str();
				if(test_next != "0") {
					printf("\n\n");
				}
			}
			std::cout << sbuf << " ";
		}
		printf("\n\n");
		return;
	}

  private:
};

int main(int argc, char** argv) {
	std::string fileName;		 //ファイル名

	//キーボード入力からファイル名を取得する
	if(argc > 1) {
		fileName = argv[1];
	} else {
		printf("Input File Name ---> ");
		getline(std::cin, fileName);
	}

	jpegImage image;
	image.inputFileName = fileName;
	image.readImageFile();
	image.getHeader();
	/* std::cout << "Size\t\t" << image.size << std::endl;
	std::cout << "Bits\t\t" << image.bits << std::endl;
	std::cout << "Width\t\t" << image.width << std::endl;
	std::cout << "Height\t\t" << image.height << std::endl;
	std::cout << "Component\t" << image.component << std::endl; */

	/* フレームヘッダの表示 */
	/* printf("\n<--- SOF --->\n");
	for(int i = 0; i < image.sof.size(); i++) {
		printf("%d : ", image.sof[i][0]);
		for(int j = 1; j < image.sof[i].size(); j++) {
			printf("%d\t", image.sof[i][j]);
		}
		printf("\n");
	} */

	/* 量子化テーブル定義を表示 */
	/* printf("\n<--- DQT --->\n");
	for(int i = 0; i < image.dqt.size(); i++) {
		printf("pqn : %d\n", image.dqt[i][0]);
		printf("tqn : %d\n", image.dqt[i][1]);
		for(int j = 2; j < image.dqt[i].size(); j++) {
			printf("%02X\t", image.dqt[i][j]);
		}
		printf("\n\n");
	} */

	/* 量子化テーブル定義を表示 */
	/* printf("\n<--- SOS --->\n");
	for(int i = 0; i < image.sos.size(); i++) {
		printf("%d : %d\t%d\n", image.sos[i][0], image.sos[i][1], image.sos[i][2]);
	}
	printf("SS : %d\n", image.ss);
	printf("SE : %d\n", image.se);
	printf("\n\n"); */

	//printf("%d\n", image.imageStart);

	//image.showAllData();
	image.getImageData();
	return 0;
}

/* char型配列に格納されたデータからバイナリデータを取り出す */
std::string getBinary(char sin, int num) {
	std::stringstream ss;
	ss << std::hex << +sin;
	std::string sout;
	std::string buf = ss.str();
	if(buf.size() > 5) {
		if(num == 1) {
			sout = ss.str().erase(0, 7);
		} else if(num == 2) {
			sout = ss.str().erase(0, 6);
			sout = sout.erase(1);
		} else {
			sout = ss.str().erase(0, 6);
		}
	} else {
		if(num == 1) {
			sout = ss.str().erase(0, 1);
		} else if(num == 2) {
			sout = ss.str().erase(1);
		} else {
			sout = ss.str();
		}
	}
	return sout;
}