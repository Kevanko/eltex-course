#include <stdio.h>
#define N 5 // arr size

void task2_check() {
    int arr[N];

    printf("Enter Array N=[%d]: ", N);

    // init
    for (int i = 0; i < N; i++){
        scanf("%d", arr + i);
    }

    // invert
    for (int i = 0; i < N / 2; i++){
        int tmp = arr[i];
        arr[i] = arr[N - 1 - i];
        arr[N - 1 - i] = tmp;
    }

    // Print
    printf("Result -> ");
    for (int i = 0; i < N; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
}