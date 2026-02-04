#include "abonent.h"

struct node * g_abonents = NULL;         

void abonent_print(struct node * abonent){
    if (abonent != NULL) {
        printf("Абонент: %s %s \n\t tel: %s\n", 
                                    abonent->data.name, 
                                    abonent->data.second_name,
                                    abonent->data.tel 
                                );
    }
    else {
        printf("Ошибка \n Абонент не найден");
    }
}

size_t get_abonents_size() {
    struct node * current = g_abonents;
    size_t count = 0;
    while(current != NULL){
        current = current->next;
        count++;
    }
    return count;
}

void abonents_clear() {
    if (!g_abonents) return;
    struct node * current = g_abonents;
    while(current != NULL) {
        struct node * next = current->next;
        free(current);
        current = next;
    }
    g_abonents = NULL;
}

struct node * create_node(struct abonent data){
    struct node * current = malloc(sizeof(struct node));
    if (!current) return NULL;
    current->data = data;
    current->next = NULL;
    current->prev = NULL;
    return current;
}

bool abonent_add(struct abonent data) {
    /* Если нет головы, создаем*/
    if (!g_abonents) {
        g_abonents = create_node(data);
        return g_abonents == NULL;
    }
    struct node * current = g_abonents;
    /* Идем в конец списка*/
    while (current->next != NULL){
        current = current->next;
    }
    /* Создаем узел и связываем его с текущим*/
    current->next = create_node(data);
    if (!current->next) 
        return 1;
    current->next->prev = current;
    return 0;
}

bool abonent_remove(char name[10]) {
    struct node * current = g_abonents;
    /* Проходимся по списку */
    while (current != NULL) {
        if(strcmp10(current->data.name, name) == 0) break;
        current = current->next;
    }
    /* Если узел существует */
    if(current != NULL){
        struct node * prev = current->prev;
        struct node * next = current->next;

        /* Если удаляется голова, меняем ее указатель на следующий узел*/
        if(g_abonents == current) {
            g_abonents = next;
        }
        /* Если не голова, связываем предыдущий узел с следующим */
        if(prev != NULL) {
            prev->next = next;
        }
        /* Если не хвост, связываем следующий узел с предыдущим */
        if(next != NULL) {
            next->prev = prev;
        }
        
        free(current);
        return 0;
    }
    printf("Ошибка\n [%s] Абонент не найден", name);
    return 1;
}

void abonent_find(char name[10]) {
    struct node * current = g_abonents;
    int strcmp_result = -1;
    bool found = false;
    /* Проходимся по списку пока не найдем имя */
    while (current != NULL) {
        strcmp_result = strcmp10(current->data.name, name);
        if(!strcmp_result) {
            abonent_print(current);
            found = true;
        }
        current = current->next;
    }
    if (found) return;
    printf("Абонент \"%s\" не найден\n", name);
}

void abonents_print() {
    struct node * current = g_abonents;
    /* Проходимся по списку */
    while (current != NULL) {
        abonent_print(current);
        current = current->next;
    }
}