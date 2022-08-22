#include <iostream>
#include "sc1.h"
using namespace std;

const int distanceMap[10][10] = 
{{0, 1, 2, 3, 4, 5, 4, 3, 2, 1},
 {1, 0, 1, 2, 3, 4, 5, 4, 3, 2},
 {2, 1, 0, 1, 2, 3, 4, 5, 4, 3},
 {3, 2, 1, 0, 1, 2, 3, 4, 5, 4},
 {4, 3, 2, 1, 0, 1, 2, 3, 4, 5},
 {5, 4, 3, 2, 1, 0, 1, 2, 3, 4},
 {4, 5, 4, 3, 2, 1, 0, 1, 2, 3},
 {3, 4, 5, 4, 3, 2, 1, 0, 1, 2},
 {2, 3, 4, 5, 4, 3, 2, 1, 0, 1},
 {1, 2, 3, 4, 5, 4, 3, 2, 1, 0}};

const int upMap[11] =
{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0};
const int downMap[11] =
{10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

main(){
	SC_input();

	const int L = SC_L;

	int pTile[SC_Nt+1][L][L];
	int parentTile[SC_Nt+1][L][L];
	for(int t = 0; t < SC_Nt+1; t++){
		for(int i = 0; i < L; i++){
			for(int j = 0; j < L; j++){
				pTile[t][i][j] = 100;
				parentTile[t][i][j] = -1;
			}
		}
	}

	pTile[0][(L-1)/2][(L-1)/2] = 0;

	for(int t = 0; t < SC_Nt; t++){
		int pT;
		int leftX;
		int rightX;
		int upperY;
		int lowerY;
		int iLj = 0;
		int q;
		for(int i = 0; i < L; i++){
			upperY = downMap[i];
			lowerY = upMap[i];
			for(int j = 0; j < L; j++){
				pT = pTile[t][i][j];
				if(pT < 23){
					leftX = downMap[j];
					rightX = upMap[j];
					if(pTile[t+1][upperY][j] > (q = pT + distanceMap[SC_C[upperY*L+j]][SC_Cr[t+1]])){
						pTile[t+1][upperY][j] = q;
						parentTile[t+1][upperY][j] = iLj;
					}
					if(pTile[t+1][lowerY][j] > (q = pT + distanceMap[SC_C[lowerY*L+j]][SC_Cr[t+1]])){
						pTile[t+1][lowerY][j] = q;
						parentTile[t+1][lowerY][j] = iLj;
					}
					if(pTile[t+1][i][leftX] > (q = pT + distanceMap[SC_C[i*L+leftX]][SC_Cr[t+1]])){
						pTile[t+1][i][leftX] = q;
						parentTile[t+1][i][leftX] = iLj;
					}
					if(pTile[t+1][i][rightX] > (q = pT + distanceMap[SC_C[i*L+rightX]][SC_Cr[t+1]])){
						pTile[t+1][i][rightX] = q;
						parentTile[t+1][i][rightX] = iLj;
					}
				}
				iLj++;
			}
		}
	}
	int minN = 0;
	int min = 100;
	for(int i = 0; i < L; i++){
		for(int j = 0; j < L; j++){
			if(min > pTile[SC_Nt][i][j] && pTile[SC_Nt][i][j] != -1){
				min = pTile[SC_Nt][i][j];
				minN = i*L + j;
			}
		}
	}
	int now = minN;

	SC_ans[SC_Nt] = now;
	for(int i = SC_Nt; i > 1; i--){
		now = parentTile[i][now/L][now%L];
		SC_ans[i-1] = now;
	}

	SC_output();
}