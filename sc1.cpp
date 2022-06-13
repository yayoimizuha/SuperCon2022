#include <iostream>
#include "sc1/sc1.h"

using namespace std;

int main() {
    SC_input();

    const int L = SC_L;
/*
	cout << "[";
	for(int i = 0; i < SC_N; i++){
		cout << SC_C[i] << ", ";
	}
	cout << "]" << endl;
	cout << "[";
	for(int i = 0; i < SC_Nt+1; i++){
		cout << SC_Cr[i] << ", ";
	}
	cout << "]" << endl;

	cout << "Hello" << endl;
*/
    bool isGoodTile[SC_Nt + 1][L][L];
    bool isJustTile[10][L][L];
    bool isRangeTile[10][L][L];

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < L; j++) {
            for (int k = 0; k < L; k++) {
                isJustTile[i][j][k] = false;
                isRangeTile[i][j][k] = false;
            }
        }
    }

    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            isJustTile[SC_C[i * L + j]][i][j] = true;
            isRangeTile[SC_C[i * L + j]][i][j] = true;
            isRangeTile[(10 + SC_C[i * L + j] + 1) % 10][i][j] = true;
            isRangeTile[(10 + SC_C[i * L + j] - 1) % 10][i][j] = true;
        }
    }
/*
	for(int i = 0; i < L; i++){
		for(int j = 0; j < L; j++){
			for(int k = 0; k < L; k++){
				cout << (isRangeTile[i][j][k] ? "#" : ".");
			}
			cout << endl;
		}
		cout << endl;
	}
*/

    for (int t = 0; t < SC_Nt + 1; t++) {
        for (int i = 0; i < L; i++) {
            for (int j = 0; j < L; j++) {
                isGoodTile[t][i][j] = false;
            }
        }
    }

    isGoodTile[0][(L - 1) / 2][(L - 1) / 2] = true;

    for (int t = 0; t < SC_Nt; t++) {
        for (int i = 0; i < L; i++) {
            for (int j = 0; j < L; j++) {
                if (isGoodTile[t][i][j]) {
                    int leftX = (L + j - 1) % L;
                    int rightX = (L + j + 1) % L;
                    int upperY = (L + i - 1) % L;
                    int lowerY = (L + i + 1) % L;
                    if (isRangeTile[SC_Cr[t + 1]][upperY][j]) {
                        isGoodTile[t + 1][upperY][j] = true;
                    }
                    if (isRangeTile[SC_Cr[t + 1]][lowerY][j]) {
                        isGoodTile[t + 1][lowerY][j] = true;
                    }
                    if (isRangeTile[SC_Cr[t + 1]][i][leftX]) {
                        isGoodTile[t + 1][i][leftX] = true;
                    }
                    if (isRangeTile[SC_Cr[t + 1]][i][rightX]) {
                        isGoodTile[t + 1][i][rightX] = true;
                    }
                }
            }
        }
        for (int i = 0; i < L; i++) {
            for (int j = 0; j < L; j++) {
//				cout << (isGoodTile[t][i][j] ? "#" : ".");
            }
//			cout << endl;
        }
//		cout << endl;
    }
    for (int p = 0; p < 1; p++)
        for (int t = SC_Nt; t > 0; t--) {
            for (int i = 0; i < L; i++) {
                for (int j = 0; j < L; j++) {
                    if (isGoodTile[t][i][j]) {
                        isGoodTile[t][i][j] = false;
                        int leftX = (L + j - 1) % L;
                        int rightX = (L + j + 1) % L;
                        int upperY = (L + i - 1) % L;
                        int lowerY = (L + i + 1) % L;
                        if (isGoodTile[t + 1][upperY][j]) {
                            isGoodTile[t][i][j] = true;
                        }
                        if (isGoodTile[t + 1][lowerY][j]) {
                            isGoodTile[t][i][j] = true;
                        }
                        if (isGoodTile[t + 1][i][leftX]) {
                            isGoodTile[t][i][j] = true;
                        }
                        if (isGoodTile[t + 1][i][rightX]) {
                            isGoodTile[t][i][j] = true;
                        }
                    }
                }
            }
            for (int i = 0; i < L; i++) {
                for (int j = 0; j < L; j++) {
//				cout << (isGoodTile[t][i][j] ? "#" : ".");
                }
//			cout << endl;
            }
//		cout << endl;
        }

/*
	for(int t = 0; t < SC_Nt; t++){
		for(int i = 0; i < L; i++){
			for(int j = 0; j < L; j++){
				cout << (isGoodTile[t][i][j] ? "#" : ".");
			}
			cout << endl;
		}
		cout << endl;
	}
*/
    int pTile[SC_Nt + 1][L][L];
    int parentTile[SC_Nt + 1][L][L];
    for (int t = 0; t < SC_Nt + 1; t++) {
        for (int i = 0; i < L; i++) {
            for (int j = 0; j < L; j++) {
                pTile[t][i][j] = 100;
                parentTile[t][i][j] = -1;
            }
        }
    }

    pTile[0][(L - 1) / 2][(L - 1) / 2] = 0;
    for (int t = 0; t < SC_Nt; t++) {
        for (int i = 0; i < L; i++) {
            for (int j = 0; j < L; j++) {
                if (isGoodTile[t][i][j]) {
                    int leftX = (L + j - 1) % L;
                    int rightX = (L + j + 1) % L;
                    int upperY = (L + i - 1) % L;
                    int lowerY = (L + i + 1) % L;
                    if (isGoodTile[t + 1][upperY][j]) {
                        if (SC_C[upperY * L + j] == SC_Cr[t + 1]) {
                            if (pTile[t + 1][upperY][j] > pTile[t][i][j]) {
                                pTile[t + 1][upperY][j] = pTile[t][i][j];
                                parentTile[t + 1][upperY][j] = i * L + j;
                            }
                        } else {
                            if (pTile[t + 1][upperY][j] > pTile[t][i][j] + 1) {
                                pTile[t + 1][upperY][j] = pTile[t][i][j] + 1;
                                parentTile[t + 1][upperY][j] = i * L + j;
                            }
                        }
                    }
                    if (isGoodTile[t + 1][lowerY][j]) {
                        if (SC_C[lowerY * L + j] == SC_Cr[t + 1]) {
                            if (pTile[t + 1][lowerY][j] > pTile[t][i][j]) {
                                pTile[t + 1][lowerY][j] = pTile[t][i][j];
                                parentTile[t + 1][lowerY][j] = i * L + j;
                            }
                        } else {
                            if (pTile[t + 1][lowerY][j] > pTile[t][i][j] + 1) {
                                pTile[t + 1][lowerY][j] = pTile[t][i][j] + 1;
                                parentTile[t + 1][lowerY][j] = i * L + j;
                            }
                        }
                    }
                    if (isGoodTile[t + 1][i][leftX]) {
                        if (SC_C[i * L + leftX] == SC_Cr[t + 1]) {
                            if (pTile[t + 1][i][leftX] > pTile[t][i][j]) {
                                pTile[t + 1][i][leftX] = pTile[t][i][j];
                                parentTile[t + 1][i][leftX] = i * L + j;
                            }
                        } else {
                            if (pTile[t + 1][i][leftX] > pTile[t][i][j] + 1) {
                                pTile[t + 1][i][leftX] = pTile[t][i][j] + 1;
                                parentTile[t + 1][i][leftX] = i * L + j;
                            }
                        }
                    }
                    if (isGoodTile[t + 1][i][rightX]) {
                        if (SC_C[i * L + rightX] == SC_Cr[t + 1]) {
                            if (pTile[t + 1][i][rightX] > pTile[t][i][j]) {
                                pTile[t + 1][i][rightX] = pTile[t][i][j];
                                parentTile[t + 1][i][rightX] = i * L + j;
                            }
                        } else {
                            if (pTile[t + 1][i][rightX] > pTile[t][i][j] + 1) {
                                pTile[t + 1][i][rightX] = pTile[t][i][j] + 1;
                                parentTile[t + 1][i][rightX] = i * L + j;
                            }
                        }
                    }
                }
            }
        }
        for (int i = 0; i < L; i++) {
            for (int j = 0; j < L; j++) {
//				if(pTile[t][i][j] == 100){
//					cout << "." << "\t";
//				}else{
//					cout << parentTile[t+1][i][j] << "\t";
//				}
            }
//			cout << endl;
        }
//		cout << endl;
    }
/*
	for(int t = SC_Nt; t < SC_Nt+1; t++){
		for(int i = 0; i < L; i++){
			for(int j = 0; j < L; j++){
				if(pTile[t][i][j] == 100){
					cout << "." << "\t";
				}else{
					cout << parentTile[t][i][j] << "\t";
				}
			}
			cout << endl;
		}
		cout << endl;
	}
*/
    int minN = 0;
    int min = 100;
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            if (isGoodTile[SC_Nt][i][j] && min > pTile[SC_Nt][i][j] && pTile[SC_Nt][i][j] != -1) {
                min = pTile[SC_Nt][i][j];
                minN = i * L + j;
            }
        }
    }
//	cout << min << endl;
    int now = minN;

//	int ans[SC_Nt];
    SC_ans[SC_Nt] = now;
    for (int i = SC_Nt; i > 1; i--) {
//		cout << now << ",";
//		cout << parentTile[i][now/L][now%L] << ", ";
        now = parentTile[i][now / L][now % L];
        SC_ans[i - 1] = now;
    }

    for (int i = 0; i < SC_Nt; i++) {
//		cout << ans[i] << endl;
    }

    SC_output();
}