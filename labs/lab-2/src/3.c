#include <stdio.h>
#define N 5 // arr size

void task3_check() {
    int arr[N][N];

    // init
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            if( (i < (N - 1)) && (j < (N - i - 1)))
                arr[i][j] = 0;
            else arr[i][j] = 1;
        }
    }
    // Print
    printf("Result -> \n");
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            printf("%d ", arr[i][j]);
    }
        printf("\n");
    }
    printf("\n");
}