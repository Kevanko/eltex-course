#include<stdio.h>

void task3_check(){
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int *arr_p = arr;
    printf("arr -> ");
    for(int i = 0; i < 10; i++){
        printf("%d ", *arr_p++);
    }
        printf("\n");
}
