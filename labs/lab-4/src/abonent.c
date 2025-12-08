#include "abonent.h"

struct abonent g_abonents[ABONENTS_SIZE] = {0};
uint g_size = 0;

void abonent_print(uint id){
    printf("[%d]\t Abonent: %s %s \n\t Tel: %s\n", 
                                    id, 
                                    g_abonents[id].name, 
                                    g_abonents[id].second_name,
                                    g_abonents[id].tel 
                                );
}


bool abonent_add(struct abonent data) {
    if(g_size >= ABONENTS_SIZE){
        printf(" Ошибка \n Справочник полон [%u из %u], удалите запись\n", g_size, ABONENTS_SIZE);
        return 1;
    }
    g_abonents[g_size] = data;
    g_size++;
    return 0;
}

bool abonent_remove(uint id) {
    if (id >= g_size || g_size == 0) {
        if(g_size == 0)
            printf(" Ошибка \n Cправочник пуст\n");
        else  
            printf(" Ошибка \n Доступные id: [0..%d] \n", g_size ? g_size - 1 : 0);
        return 1;
    }
    // Swap element 
    if(id != g_size - 1) {
        g_abonents[id] = g_abonents[g_size - 1];
    }

    g_abonents[g_size - 1] = (struct abonent){ 0 }; 
    g_size--;
    return 0;
}

void abonent_find(char name[10]) {
    bool found = false;
    for(uint id = 0; id < g_size; id++) {
        if (!strcmp10(g_abonents[id].name, name)){
            abonent_print(id);
            found = true;
        }
    }
    if(!found){
        printf("Абонент \"%s\" не найден\n", name);
    }
}

void abonent_printALL() {
    for(uint id = 0; id < g_size; id++) {
        abonent_print(id);
    }
}