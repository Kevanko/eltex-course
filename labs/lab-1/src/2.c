#include <string.h>
#include "task.h"

#define MAX_BITS (sizeof(int) * 8)
#define BUFFER_SIZE (MAX_BITS + 1)

void task2(int num, char* result) {
    if(num == 0) {
        strcpy(result, "0");
        return;
    }
    unsigned int i = 0;

    // Устанавливаем маску на старший бит
    unsigned int mask = 1U << (MAX_BITS - 1);

    // Записываем биты в строку
    while(i < MAX_BITS){
        result[i++] = (num & mask) ? '1' : '0';
        mask = mask >> 1;
    }
    result[i] = '\0';
}


void task2_check(){
    int num; 
    printf("enter -num: ");
    scanf("%d", &num);

    char* result = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    task2(num, result);

    printf("result: %s\n", result);
    free(result);
}