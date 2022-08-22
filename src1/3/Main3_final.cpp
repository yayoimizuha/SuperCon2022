#include <iostream>
#include <time.h>
#include "sc1.h"
using namespace std;

int SC_distance(int c){
	int a,b;
	a=abs(c);
	b=10-a;
	return (a > b) ? b : a;
};

main(){
	SC_input();

	const int L = SC_L;

	bool isJustTile[10][L][L];
	bool isRangeTile[10][L][L];

	for(int i = 0; i < 10; i++){
		for(int j = 0; j < L; j++){
			for(int k = 0; k < L; k++){
				isJustTile[i][j][k] = 0;
				isRangeTile[i][j][k] = 0;
			}
		}
	}

	for(int i = 0; i < L; i++){
		for(int j = 0; j < L; j++){
			isJustTile[SC_C[i*L+j]][i][j] = true;
			isRangeTile[SC_C[i*L+j]][i][j] = true;
			isRangeTile[(10+SC_C[i*L+j]+1)%10][i][j] = true;
			isRangeTile[(10+SC_C[i*L+j]-1)%10][i][j] = true;
		}
	}




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
				int leftX = (L + j - 1) % L;
				int rightX = (L + j + 1) % L;
				int upperY = (L + i - 1) % L;
				int lowerY = (L + i + 1) % L;
				if(SC_C[upperY*L+j] == SC_Cr[t+1]){
					if(pTile[t+1][upperY][j] > pTile[t][i][j]){
						pTile[t+1][upperY][j] = pTile[t][i][j];
						parentTile[t+1][upperY][j] = i*L + j;
					}
				}else{
					if(pTile[t+1][upperY][j] > pTile[t][i][j] + SC_distance(SC_C[upperY*L+j] - SC_Cr[t+1])){
						pTile[t+1][upperY][j] = pTile[t][i][j] + SC_distance(SC_C[upperY*L+j] - SC_Cr[t+1]);
						parentTile[t+1][upperY][j] = i*L + j;
					}
				}
				if(SC_C[lowerY*L+j] == SC_Cr[t+1]){
					if(pTile[t+1][lowerY][j] > pTile[t][i][j]){
						pTile[t+1][lowerY][j] = pTile[t][i][j];
						parentTile[t+1][lowerY][j] = i*L + j;
					}
				}else{
					if(pTile[t+1][lowerY][j] > pTile[t][i][j] + SC_distance(SC_C[lowerY*L+j] - SC_Cr[t+1])){
						pTile[t+1][lowerY][j] = pTile[t][i][j] + SC_distance(SC_C[lowerY*L+j] - SC_Cr[t+1]);
						parentTile[t+1][lowerY][j] = i*L + j;
					}
				}
				if(SC_C[i*L+leftX] == SC_Cr[t+1]){
					if(pTile[t+1][i][leftX] > pTile[t][i][j]){
						pTile[t+1][i][leftX] = pTile[t][i][j];
						parentTile[t+1][i][leftX] = i*L + j;
					}
				}else{
					if(pTile[t+1][i][leftX] > pTile[t][i][j] + SC_distance(SC_C[i*L+leftX] - SC_Cr[t+1])){
						pTile[t+1][i][leftX] = pTile[t][i][j] + SC_distance(SC_C[i*L+leftX] - SC_Cr[t+1]);
						parentTile[t+1][i][leftX] = i*L + j;
					}
				}
				if(SC_C[i*L+rightX] == SC_Cr[t+1]){
					if(pTile[t+1][i][rightX] > pTile[t][i][j]){
						pTile[t+1][i][rightX] = pTile[t][i][j];
						parentTile[t+1][i][rightX] = i*L + j;
					}
				}else{
					if(pTile[t+1][i][rightX] > pTile[t][i][j] + SC_distance(SC_C[i*L+rightX] - SC_Cr[t+1])){
						pTile[t+1][i][rightX] = pTile[t][i][j] + SC_distance(SC_C[i*L+rightX] - SC_Cr[t+1]);
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