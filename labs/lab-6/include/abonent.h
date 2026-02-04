/**
* @file abonent.h
* @brief Заголовочный файл справочника абонентов
*
* Динамическое выделение памяти и способ хранения -
* Двухсвязный список
*/

#ifndef ABONENT_H
#define ABONENT_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utils.h"

/**
 *  @brief Структура абонента
 * 
 *  Хранит имя, фамилию и телефон (char [10]) 
 *  с учетом '\0' всего 9 символов на каждое поле
*/
struct abonent {
    char name[10];
    char second_name[10];
    char tel[10];
}; 

/**
 *  @brief Узел с абонентом
 * 
 *  Двухсвязный список, хранит абонента, указатель на предыдущий узел
 *  и указатель на следующий
*/
struct node {
    struct abonent data;
    struct node *next;
    struct node *prev;
};


/**
 * @brief Главная функция интерфейса справочника
 * 
 * Запускает цикл меню: добавление, удаление, вывод в консоль
 * Использует 'system("clear")' для очистки и лучшей читаемости
 * Считывает значения с помощью 'scanf()' после которого для безопасности очищает ввод 'clear_input()'
 * 
*/
void abonent_handbook();

/**
 * @brief Добавить абонента в справочник
 * @param[in] data структура с данными нового абонента
 * @return 'false' при успехе, 'true' при ошибке выделения памяти
 * 
 * @note Выделяет память под узел с помощью malloc'.
*/
bool abonent_add(struct abonent data);

/**
 * @brief Удалить абонента по имени
 * @param[in] name Имя абонента (char[10])
 * @return 'false' при успехе, 'true' при ошибке
 * 
 * @note Проходит по узлам до конца и сравнивает с помощью strcmp10 имена
 * @see strcmp10
*/
bool abonent_remove(char name[10]);

/**
 * @brief Найти всех абонентов по имени
 * @param[in] name искомое имя (до 10 байт)
 * 
 * @note Сравнение побайтовое (10 байт), не зависит от '\0'. Использует 'strcmp10()'.
 * @see strcmp10
*/
void abonent_find(char name[10]);

/**
 * @brief Вывести всех абонентов
 * 
 * Вызывает 'abonent_print()' для каждого элемента 'g_abonents'.
*/
void abonents_print();

/**
 * @brief Получить число узлов
 * 
 * Проходится по узлам 'g_abonents' и подсчитывает их О(n).
*/
size_t get_abonents_size();

/**
 * @brief Очистка памяти
 * 
 * Меняет голову на NULL и очищает 'free()' все узлы
*/
void abonents_clear();

#endif // ABONENT_H