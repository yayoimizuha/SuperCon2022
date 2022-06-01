# include <stdio.h>
# include <stdlib.h>
# include "SFMT-src-1.5.1/SFMT.h"
# include "sc3_problem.h"

void gene_tile();
void gene_robot_path();

int main(int argc, char *argv[]){

  int seed;

  seed=atoi(argv[1]);

  /*  printf("#seed= %d \n",seed);*/

  int i,ix,iy;
  i=0;
  for (ix=0; ix< SC_L; ix++){
    for (iy=0; iy< SC_L; iy++){
      ps[ix][iy]=i;
      psx[i]=ix;
      psy[i]=iy;
      i=i+1; 
    };
  };

  idx[0]=1;
  idx[1]=0;
  idx[2]=-1;
  idx[3]=0;
  
  idy[0]=0;
  idy[1]=1;
  idy[2]=0;
  idy[3]=-1;

  sfmt_init_gen_rand( &sfmt, seed);

  int iflag;
  int nsample;

  iflag=1;
  nsample=0;
  while (iflag){
 
    gene_tile();

    /* check degeneracy of the colors among neighbours*/
    /* exclude samples with the degeneracy */

    int k,l;
    int ix0,iy0;
    int tcn[4];

    int n_degeneracy;

    n_degeneracy=0;
    for (i=0; i < SC_N; i++){
      ix0=psx[i];
      iy0=psy[i];
      for (k=0; k< 4; k++){
	tcn[k]=SC_C[ps[(ix0+idx[k]+SC_L)%SC_L][(iy0+idy[k]+SC_L)%SC_L]];
      };
      for (k=0; k< 4; k++){
	for (l=k+1; l< 4; l++){
	  if (tcn[k] == tcn[l]){
	    n_degeneracy=n_degeneracy+1;
	  };
	};
      };
    };

    nsample=nsample+1;
    if (n_degeneracy==0){
      iflag=0;
    };
  };

  /*  printf("#nsample= %d \n",nsample);*/
  /*  printf("#tile color \n");*/
  for (i=0; i < SC_N; i++){
    /*          printf("%d %d\n",i,SC_C[i]);*/
                printf("%d\n",SC_C[i]);
  };

  gene_robot_path();

  /*  printf("#robot report \n");  */
  int n,t0;
  for (n=1; n<= SC_Nt; n++){
    i=path0[n];
    /* without noise*/
    SC_Cr[n]=SC_C[path0[n]];
    /*    printf("%d %d \n",n,SC_Cr[n]);*/
    printf("%d \n",SC_Cr[n]);
  };

  return 0;

};


void gene_tile(){

  double r;

  int i;
  for (i=0; i < SC_N; i++){
    r=sfmt_genrand_res53(&sfmt);
    SC_C[i]=(int) (r*SC_Nq);
  };

};

void gene_robot_path(){

  double r;
  int n,ix,iy;
  int k;

  ix=(SC_L-1)/2;
  iy=(SC_L-1)/2;
  path0[0]=ps[ix][iy];

  for (n=1; n <= SC_Nt; n++){
    r=sfmt_genrand_res53(&sfmt);
    k=(int) (r*4);
    ix=(ix+idx[k]+SC_L)%SC_L;
    iy=(iy+idy[k]+SC_L)%SC_L;
    path0[n]=ps[ix][iy];
  };
};
