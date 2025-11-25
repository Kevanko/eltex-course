#include <string.h>
#include "task.h"

#define MAX_BITS (sizeof(unsigned int) * 8)
#define BUFFER_SIZE (MAX_BITS + 1)

void task1(unsigned int num, char* result) {
    if(num == 0) {
        strcpy(result, "0");
        return;
    }
    unsigned int i = 0;

    // Устанавливаем маску на старший бит
    unsigned int mask = 1U << (MAX_BITS - 1);

    // Пропускаем ведущие нули
    while(!(mask & num)){
        mask = mask >> 1;
    }
    // Записываем биты в строку
    while(mask != 0){
        result[i++] = (num & mask) ? '1' : '0';
        mask = mask >> 1;
    }
    result[i] = '\0';
}


void task1_check(){
    unsigned int num; 
    printf("enter num: ");
    scanf("%u", &num);

    char* result = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    task1(num, result);

    printf("result: %s\n", result);
    free(result);
}