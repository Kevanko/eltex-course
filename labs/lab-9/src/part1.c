#include <stdio.h>
#define STR_SIZE 16
#define FILE_NAME "output.txt"

/* П.1: записать строку в файл, потом вывести её с конца посимвольно */

int main(void) {
    char str[STR_SIZE] = "String from file";
    FILE *file;

    file = fopen(FILE_NAME, "w"); /* запись */
    if (file == NULL) {
        perror("fopen write");
        return 1;
    }

    fwrite(str, sizeof(char), STR_SIZE, file);
    fclose(file);

    file = fopen(FILE_NAME, "r"); /* чтение */
    if (file == NULL) {
        perror("fopen read");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    /* идём с конца файла */
    for (size_t i = 0; i < file_size; i++) {
        fseek(file, (long)(file_size - i - 1), SEEK_SET);
        char c = fgetc(file);
        printf("%c", c);
    }
    printf("\n");
    fclose(file);
    
    return 0;
}