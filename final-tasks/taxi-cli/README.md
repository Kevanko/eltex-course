# Taxi CLI

## Задание

CLI управляет процессами-водителями.

Команды:

```text
create_driver
send_task <pid> <task_timer>
get_status <pid>
get_drivers
exit
```

`create_driver` создает новый процесс через `fork`.
PID созданного процесса используется как номер водителя.

`send_task` отправляет задачу водителю.
Если водитель свободен, он становится `Busy` на указанное количество секунд.
Если водитель уже занят, он отвечает `Busy <task_timer>`.

`get_status` показывает состояние одного водителя.
`get_drivers` показывает состояние всех водителей, созданных этим CLI.

## Как сделано

CLI и driver общаются через два `pipe`:

```text
CLI -> driver
driver -> CLI
```

Driver ждет команды через `poll`.
Таймер задачи сделан через `alarm`.
Когда время задачи заканчивается, driver получает `SIGALRM` и снова становится `Available`.

## Сборка

```bash
make
```

## Запуск

```bash
make run
```

## Пример работы

```text
$ ./bin/taxi-cli
Commands:
create_driver
send_task <pid> <task_timer>
get_status <pid>
get_drivers
exit
taxi> create_driver
Created driver 25233
taxi> send_task 25233 2
Task accepted
taxi> get_status 25233
Busy 2
taxi> send_task 25233 5
Busy 2
taxi> get_drivers
25233: Busy 2
taxi> get_status 25233
Available
taxi> exit
```
