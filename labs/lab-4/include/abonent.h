#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utils.h"

#define ABONENTS_SIZE 100

struct abonent {
    char name[10];
    char second_name[10];
    char tel[10];
}; 
extern struct abonent g_abonents[ABONENTS_SIZE];
extern uint g_size;

void abonent_handbook();

bool abonent_add(struct abonent data);
bool abonent_remove(uint id);
void abonent_find(char name[10]);
void abonent_printALL();