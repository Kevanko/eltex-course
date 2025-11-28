#include <stdio.h>
#define N 5 // N^2 arr size


void task4_check() {
    int arr[N][N] = {0};
    int row = 0, col = 0;
    int counter = 1;
    int steps = N;

    while (counter <= N * N) {
        // right
        for (int j = 0; j < steps; j++) arr[row][col++] = counter++;
        col--; row++;

        // down
        for (int j = 0; j < steps - 1; j++) arr[row++][col] = counter++;
        row--; col--;

        // left
        for (int j = 0; j < steps - 1; j++) arr[row][col--] = counter++;
        col++; row--;

        // up
        for (int j = 0; j < steps - 2; j++) arr[row--][col] = counter++;
        row++; col++;

        steps -= 2;
    }

    // Print
    printf("Result -> \n");
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            printf("%d\t", arr[i][j]);
    }
        printf("\n");
    }
    printf("\n");
}