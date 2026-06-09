#include <stdio.h>
#define TAM 100

void_suma(int a[][TAM], int b[][TAM], int c[][TAM], int n, int m)
    int *ptrA = &a[0][0];
    int *ptrB = &b[0][0];
    int *ptrc = &c[0][0];

    for(i=0: i < n; i++){
        for(int j = 0; j <m; j++){
            *(ptrc + i*m + j) = (ptrA + i*m + j) + *(ptrB + i*m + j);
        }
    
    }