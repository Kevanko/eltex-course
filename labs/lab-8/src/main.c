#include <stdio.h>
#include "calc.h"

int main() {
    int choice, a, b;

    while (1) {
        printf("\n1) Сложение\n2) Вычитание\n3) Умножение\n4) Деление\n5) Выход\nВыбор: ");
        scanf("%d", &choice);

        if (choice == 5) break;
        if (choice < 1 || choice > 4) {
            printf("Неверный пункт!\n");
            continue;
        }

        printf("Введите два числа: ");
        scanf("%d %d", &a, &b);

        switch (choice) {
            case 1: 
                printf("\nРезультат: %d\n", add(a, b)); 
                break;
            case 2: 
                printf("\nРезультат: %d\n", sub(a, b)); 
                break;
            case 3: 
                printf("\nРезультат: %d\n", mul(a, b)); 
                break;
            case 4: 
                printf("\nРезультат: %d\n", div(a, b)); 
                break;
            default: 
                break;
        }
    }
    return 0;
}