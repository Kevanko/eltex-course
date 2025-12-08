#include "abonent.h"

void print_menu_tabs(){
    printf("----Абонентский справочник [%d из %d]----\n", g_size, ABONENTS_SIZE);
    printf("1) Добавить абонента \n");
    printf("2) Удалить абонента \n");
    printf("3) Поиск абонентов по имени \n");
    printf("4) Вывод всех записей \n");
    printf("5) Выход \n");
}

void print_error_id(){
    printf("Ошибка, некорректный номер (1 - 5)\n");
}

void abonent_handbook(){
    uint menu_id = 0;
    system("clear"); // Clear terminal

    while(menu_id != 5) {
        print_menu_tabs();

        scanf("%u", &menu_id);
        system("clear"); // Clear terminal

        switch (menu_id)
        {
        case 1: // ADD ABONENT
            struct abonent new_abonent;

            printf("name[10]: " );         scanf("%9s", new_abonent.name);
            printf("second_name[10]: " );  scanf("%9s", new_abonent.second_name);
            printf("tel[10]: " );          scanf("%9s", new_abonent.tel);

            system("clear"); // Clear terminal

            if(!abonent_add(new_abonent)) {
                printf(" Абонент \"%s\" был добавлен [id: %u]\n", new_abonent.name, g_size - 1); // Log
            }

            break;

        case 2: // REMOVE ABONENT
            uint id;
            printf("Введите id абонента: " ); scanf("%u", &id);

            system("clear"); // Clear terminal

            if(!abonent_remove(id)) {
                printf(" Абонент [id: %u] был удален\n", id); // Log
            }

            break;
        case 3:
            char name[10];
            printf("Введите имя абонента: " ); scanf("%9s", name);

            system("clear"); // Clear terminal
            abonent_find(name);
            break;
        case 4:
            abonent_printALL();
            break;
        case 5:
            break;
        default:
            print_error_id();
            break;
        }

    }
}