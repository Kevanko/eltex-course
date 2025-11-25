#include <string.h>
#include "task.h"

size_t task3(unsigned int num) {
    size_t count = 0;

    while(num != 0){
        count += (num & 1U) ? 1 : 0;
        num = num >> 1;
    }
    return count;
}


void task3_check(){
    unsigned int num; 
    printf("enter num: ");
    scanf("%u", &num);

    printf("count <1>: %zu\n", task3(num));
}