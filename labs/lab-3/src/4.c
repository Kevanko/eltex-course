#include <stdio.h>

#define STR_SIZE 32

char* find_substr(char * str, char * substr){
    if (*substr == '\0') return str;

   for (char *start = str; *start != '\0'; start++){
        char * sub_copy = substr;
        char * start_copy = start;

        while(*start_copy != '\0' && *sub_copy != '\0' && *sub_copy == *start_copy){
            start_copy++;
            sub_copy++;
        }   
        if(*sub_copy == '\0') return start;
   }
   
    return NULL;
}

void task4_check(){
    char str[STR_SIZE];
    char substr[STR_SIZE];

    printf("enter str[%d]: ", STR_SIZE);
    scanf("%31s", str);
    printf("\nenter substr[%d]: ", STR_SIZE);
    scanf("%31s", substr);

    char* result = find_substr(str, substr);
    printf("Found: %s\n", result? result : "(NULL)");
}
