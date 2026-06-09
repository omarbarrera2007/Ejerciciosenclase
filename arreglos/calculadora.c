#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
float suma(float a, float b){
    float c;
    c = a + b;
    return c;
}

float resta(float a, float b){
    return a-b;
}

float mul(float a, float b){
    return a*b;
}
float my_div(float a, float b){
    if(b==0){
        printf("La división con cero no existe\n");
        return 0;
    }
    return a/b;
}
void menu(){
    printf("1.Suma\n");
    printf("3.Resta\n");
    printf("3.Multiplicación\n");
    printf("4.Divisón\n");
    printf("5.Salir\n");
    
}

int main(){
    float x,y,z;
    int op;

    while(true){
        menu();//Llamado a la funcion menú
        printf("Selecciona una opción");
        scanf("%d",&op);
        if(op == 5) return 0;

        printf("Ingresa el primer valor: ");
        scanf("%f",&x);
         
        printf("Ingresa el segundo valor: ");
        scanf("%f",&y);

    
        switch(op){
            case 1:
                z=suma(x, y);
                printf("%f + %f = %f\n", x, y, z);
                break;
            case 2:
                z=resta(x, y);
                printf("%f - %f = %f\n", x, y, z);
                break;
            case 3:
            z=mul(x, y);
                printf("%f * %f = %f\n", x, y, z);
                break;

            case 4:
            z=my_div(x, y);
             if(z != INFINITY)
                printf("%f / %f = %f\n", x, y, z);
                break;
            case 5:
                return 0;
            default:
            printf("Opción no valida\n");
        }
        getchar();
    system("clear");
    }

}

