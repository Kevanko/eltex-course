#pragma once
#include "stdio.h"
#include "stdlib.h"

#define MAX_BITS (sizeof(int) * 8)
#define BUFFER_SIZE (MAX_BITS + 1)

void task1(unsigned int num, char* result);
void task1_check();

void task2(int num, char* result);
void task2_check();

size_t task3(unsigned int num);
void task3_check();

unsigned int task4(unsigned int num, unsigned char byte);
void task4_check();