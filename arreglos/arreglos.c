#include <stdio.h>

int main(){
    int array[5];
    int *ptr;

    ptr = array;
    printf("%p\n", &array[0]);
    printf("%p\n", ptr);

    for(int i=0; i < 5; i++){
        *(ptr+i) = i+1;
    }

    for(int i=0; i < 5; i++){
    printf("%d %p\n", *(ptr+i),(ptr+i));
    }

return 0;
}