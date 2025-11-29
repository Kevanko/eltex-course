#include <stdio.h>
#define BYTE_NUM 3

void task1_check(){
    unsigned int num; 
    char * nump = (char*)&num;
    unsigned char byte;

    printf("enter num: ");
    scanf("%u", &num);
    printf("enter byte [AF, F0, 14]: ");
    scanf("%hhx", &byte);

    *(nump + BYTE_NUM - 1) = byte;

    printf("result: %u\n", num);

    // Debug
    printf("Debug bytes (little-endian): \nnum-> ");
    for(int i = 0; i < 4; i++){
        printf("%hhx ", *(nump + i));
    }
    printf("\n");
}