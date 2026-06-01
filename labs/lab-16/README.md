# 16 - Сокеты

## Задание 1

Клиент отправляет серверу строку `hello!` и ждет ответ.
Сервер получает строку, выводит ее и отправляет ответ `hi!`.

Варианты:

1) `AF_LOCAL + SOCK_DGRAM`

2) `AF_LOCAL + SOCK_STREAM`

3) `AF_INET + SOCK_DGRAM`

4) `AF_INET + SOCK_STREAM`

## Задание 2

Клиент запрашивает текущее время у сервера.

Варианты серверов:

1) На каждого клиента создается отдельный поток.

2) При запуске создается пул потоков обслуживания.

3) Шаблон потребитель/производитель с очередью клиентов.

4) Один мультипротокольный сервер обслуживает TCP и UDP через `poll`.

## Задание 3

Передача набора произвольных символов:

1) `broadcast`

2) `multicast`

## Raw sockets

Обычный UDP сервер получает строку, выводит ее, меняет первый символ и отправляет строку обратно.

Клиенты:

1) Добавляет UDP заголовок.

2) Добавляет IP заголовок.

3) Добавляет канальный заголовок.

Raw-клиенты нужно запускать с правами root.

Для сетевых примеров используется порт `7777`.
Для multicast используется адрес `224.0.0.2`.

## Структура проекта

```text
lab-16/
├── Makefile
├── README.md
├── part-1
│   ├── 1
│   ├── 2
│   ├── 3
│   └── 4
├── part-2
│   ├── 1
│   ├── 2
│   ├── 3
│   └── 4
├── part-3
│   ├── 1
│   └── 2
└── part-4
    ├── 1
    ├── 2
    ├── 3
    └── server.c
```

## Сборка

Собрать все программы:

```bash
make
```

Очистить бинарные файлы:

```bash
make clean
```

## Пример работы 1 задания

### 1 задание

```text
Terminal 1:
$ make run-1-server
Received: hello!

Terminal 2:
$ make run-1-client
Received: hi!
```

### 2 задание

```text
Terminal 1:
$ make run-2-server
Received: hello!

Terminal 2:
$ make run-2-client
Received: hi!
```

### 3 задание

```text
Terminal 1:
$ make run-3-server
Received: hello!

Terminal 2:
$ make run-3-client
Received: hi!
```

### 4 задание

```text
Terminal 1:
$ make run-4-server
Received: hello!

Terminal 2:
$ make run-4-client
Received: hi!
```

## Пример работы 2 задания

### 1 задание

```text
Terminal 1:
$ make run-2-1-server
Thread-per-client server started on port 7777

Terminal 2:
$ make run-2-1-client
Current time: 2026-06-01 11:12:21
```

### 2 задание

```text
Terminal 1:
$ make run-2-2-server
Pool listener started on port 7777
worker 1 is free

Terminal 2:
$ make run-2-2-client
Current time: 2026-06-01 11:12:22
```

### 3 задание

```text
Terminal 1:
$ make run-2-3-server
Producer/consumer server started on port 7777
worker 1 handles client

Terminal 2:
$ make run-2-3-client
Current time: 2026-06-01 11:12:22
```

### 4 задание

TCP клиент:

```text
Terminal 1:
$ make run-2-4-server
Multiprotocol server started on tcp/udp port 7777

Terminal 2:
$ make run-2-4-tcp-client
TCP time: 2026-06-01 11:12:22
```

UDP клиент:

```text
Terminal 1:
$ make run-2-4-server
Multiprotocol server started on tcp/udp port 7777

Terminal 2:
$ make run-2-4-udp-client
UDP time: 2026-06-01 11:12:22
```

## Пример работы 3 задания

### 1 задание

```text
Terminal 1:
$ make run-3-1-server
Received: broadcast message

Terminal 2:
$ make run-3-1-client
Received: broadcast: broadcast message
```

### 2 задание

```text
Terminal 1:
$ make run-3-2-server
Received: multicast message

Terminal 2:
$ make run-3-2-client
Received: multicast: multicast message
```

## Пример работы Raw sockets

### 1 задание

```text
Terminal 1:
$ make run-4-server-raw
Received: raw udp header
Modified: Raw udp header

Terminal 2:
$ make run-4-1-client
Received: Raw udp header
```

### 2 задание

```text
Terminal 1:
$ make run-4-server-raw
Received: raw ip header
Modified: Raw ip header

Terminal 2:
$ make run-4-2-client
Received: Raw ip header
```

### 3 задание

```text
Terminal 1:
$ make run-4-server-raw
Received: raw ethernet header
Modified: Raw ethernet header

Terminal 2:
$ make run-4-3-client
Received: Raw ethernet header
```
