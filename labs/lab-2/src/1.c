#include <stdio.h>
#define N 3

void task1_check() {
    int arr[N][N];
    int counter = 1;

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            arr[i][j] = counter++;
            printf("%d\t", arr[i][j]);
        }
        printf("\n");
    }

}