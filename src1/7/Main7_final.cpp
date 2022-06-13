#include <iostream>
#include <time.h>
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

    clock_t start = clock();

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
		for(int i = 0; i < L; i++){
			for(int j = 0; j < L; j++){
				if(pTile[t][i][j] < 20){
					int leftX = downMap[j];
					int rightX = upMap[j];
					int upperY = downMap[i];
					int lowerY = upMap[i];
					if(pTile[t+1][upperY][j] > pTile[t][i][j] + distanceMap[SC_C[upperY*L+j]][SC_Cr[t+1]]){
						pTile[t+1][upperY][j] = pTile[t][i][j] + distanceMap[SC_C[upperY*L+j]][SC_Cr[t+1]];
						parentTile[t+1][upperY][j] = i*L + j;
					}
					if(pTile[t+1][lowerY][j] > pTile[t][i][j] + distanceMap[SC_C[lowerY*L+j]][SC_Cr[t+1]]){
						pTile[t+1][lowerY][j] = pTile[t][i][j] + distanceMap[SC_C[lowerY*L+j]][SC_Cr[t+1]];
						parentTile[t+1][lowerY][j] = i*L + j;
					}
					if(pTile[t+1][i][leftX] > pTile[t][i][j] + distanceMap[SC_C[i*L+leftX]][SC_Cr[t+1]]){
						pTile[t+1][i][leftX] = pTile[t][i][j] + distanceMap[SC_C[i*L+leftX]][SC_Cr[t+1]];
						parentTile[t+1][i][leftX] = i*L + j;
					}
					if(pTile[t+1][i][rightX] > pTile[t][i][j] + distanceMap[SC_C[i*L+rightX]][SC_Cr[t+1]]){
						pTile[t+1][i][rightX] = pTile[t][i][j] + distanceMap[SC_C[i*L+rightX]][SC_Cr[t+1]];
						parentTile[t+1][i][rightX] = i*L + j;
					}
				}
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