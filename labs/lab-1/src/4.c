#include <string.h>
#include "task.h"

unsigned int task4(unsigned int num, unsigned char byte) {
    return (num & 0xFF00FFFF) | (byte << 2 * 8);
}


void task4_check(){
    unsigned int num; 
    unsigned char byte;

    printf("enter num: ");
    scanf("%u", &num);
    printf("enter byte [AF, F0, 14]: ");
    scanf("%hhx", &byte);

    unsigned int result = task4(num, byte);
    printf("result: %u\n", result);

    // Debug
    char* str = (char*)malloc(sizeof(char) * 32);
    task2(num, str);
    printf("before: %s\n", str);

    task2(result, str);
    printf("after:  %s\n", str);

    free(str);
}