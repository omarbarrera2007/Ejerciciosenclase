#include <stdio.h
#define TAM 100

void print_dir_array(float array[][TAM], int nr, int nc){
float *ptr = &array[0][0];
for (int i=0; i < nr*nc; i++){
printf("%p \n", (ptr + i));
}
}

void print_array(float array[][TAM], int nr, int nc){
float *ptr= &array[0][0];
printf("[\n ");
for(int i =0; i < nr*nc; i++){
printf("%f ", *(ptr+i));
if((i+1) % nc ==0) printf("\n");}
printf("]\n");
}

int main(){
float matriz[][TAM]={1.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.1f}
float *ptr;
*ptr = &matriz[0][0];

}
