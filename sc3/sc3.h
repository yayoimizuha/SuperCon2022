#include<stdio.h>
#include<time.h>

# define SC_L 11
# define SC_N 121
# define SC_Nq 100
# define SC_Nt 100

/*int tc[SC_N];*/
int SC_C[SC_N];

/*int tc0[SC_Nt+1];*/
int SC_Cr[SC_Nt+1];

int SC_ans[SC_Nt+1];

clock_t SC_starttime,SC_endtime;

void SC_input(){
	int i;
	for(i=0; i< SC_N; i++){scanf("%d",&SC_C[i]);};
	int n;
	for(n=1; n<= SC_Nt; n++){scanf("%d",&SC_Cr[n]);};
	SC_starttime=clock();
}

void SC_output(){
	SC_endtime=clock();
	int n;

	for(n=1; n<= SC_Nt; n++){printf("%d\n",SC_ans[n]);};
	/*        for(n=1; n<= SC_Nt; n++){printf("%d %d\n",SC_ans[n],SC_C[SC_ans[n]]);};*/
	printf("# elapsed time (sec) = %f \n", (double) (SC_endtime-SC_starttime)/CLOCKS_PER_SEC);

}






