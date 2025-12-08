/**
 * @file utils.h
 * @brief Вспомогательные функции
*/
#pragma once

typedef unsigned int uint; ///< Заменяется на unsigned int

/**
 * @brief Сравнение двух 10-байтных буферов
 * @param[in] first  первый буфер
 * @param[in] second второй буфер
 * @return 0 если равны, иначе разность первых несовпадающих байт
 */
int strcmp10(const char first[10], const char second[10]);

/**
 * @brief Очистить буфер stdin до символа новой строки
 * 
 * Используется после `scanf()` для удаления остатков строки (включая '\n').
 */
void clear_input();