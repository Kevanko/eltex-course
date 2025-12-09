/**
* @file abonent.h
* @brief Заголовочный файл справочника абонентов
*/

#ifndef ABONENT_H
#define ABONENT_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utils.h"

#define ABONENTS_SIZE 100 ///< Максимальное количество абонентов

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

extern struct abonent g_abonents[ABONENTS_SIZE];    ///< Глобальный массив абонентов
extern uint g_size;                                 ///< Текущее количество абонетов

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
 * @return 'false' при успехе, 'true' при переполнении
 * 
 * @note Проверяет: 'g_size < ABONENTS_SIZE'. При успехе копирует 'data' в 'g_abonents[g_size]'.
*/
bool abonent_add(struct abonent data);

/**
 * @brief Удалить абонента по ID
 * @param[in] id индекс абонента (0..g_size-1)
 * @return 'false' при успехе, 'true' при ошибке (ID вне диапазона)
 * 
 * @note Удаление за O(1): заменяет 'id' на последний элемент, последний зануляется.
*/
bool abonent_remove(uint id);

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
 * Вызывает 'abonent_print()' для каждого элемента 'g_abonents[0..g_size-1]'.
*/
void abonent_printALL();

#endif // ABONENT_H