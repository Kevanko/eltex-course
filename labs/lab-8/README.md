# 8 - Динамические библиотеки

## Задание
Переписать программу из задания на статические библиотеки так чтобы
применялась динамическая библиотека. Т. е. чтобы весь функционал
находился в динамической библиотеке. 

При запуске на экран выводится текстовое меню:
```bash
1) Сложение
2) Вычитание
3) Умножение
4) Деление
5) Выход
```

## Структура проекта
```bash
├── bin              # Обьектные файлы, main файл и библиотека libcalc.so
├── include
│   └── calc.h       # Прототипы функций (add, sub, mul, div)
├── Makefile         # Сборка объектов, библиотеки и линковка
├── README.md
└── src
    ├── main.c       # Интерфейс пользователя и логика меню
    ├── add.c        # Модуль сложения
    ├── sub.c        # Модуль вычитания
    ├── mul.c        # Модуль умножения
    └── div.c        # Модуль деления
```

## Сборка и запуск
```bash
make run             # Сборка и запуск
make clean           # Удаление объектных файлов и бинарника
```


## Проверка

> Проверка через ldd - список всех зависимостей
```bash
ldd bin/main

    linux-vdso.so.1 (0x00007510a054c000)
    libcalc.so => ./bin/libcalc.so (0x00007510a053c000)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007510a0200000)
    /lib64/ld-linux-x86-64.so.2 (0x00007510a054e000)
```

## Шаги сборки

> Создание объектных файлов с флагом -fPIC fPIC (Position Independent Code) - генерирует код, который может работать по любому адресу в памяти
```bash
gcc -fPIC -c src/add.c -o bin/add.o -I include
gcc -fPIC -c src/sub.c -o bin/sub.o -I include
gcc -fPIC -c src/mul.c -o bin/mul.o -I include
gcc -fPIC -c src/div.c -o bin/div.o -I include
```

> Создание самой динамической библиотеки (.so) Вместо ar используем gcc с флагом -shared
```bash
gcc -shared bin/add.o bin/sub.o bin/mul.o bin/div.o -o bin/libcalc.so
```

> Линковка
```bash
gcc src/main.c -I include -L bin -lcalc -o bin/main
```
