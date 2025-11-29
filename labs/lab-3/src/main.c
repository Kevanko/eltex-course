#include "task.h"

int main(){
    unsigned int task_num; 
    printf("enter task num (1-4): ");
    scanf("%u", &task_num);

    switch (task_num)
    {
    case 1:
        task1_check();
        break;
    case 2:
        task2_check();
        break;
    case 3:
        task3_check();
        break;  
    case 4:
        task4_check();
        break; 

    default:
        printf("Task: 1 - 4\n");
        break;
    }
    return 0;
}