# Raw echo

## Задание

Написать echo-client и echo-server на raw сокетах.

Сервер отвечает клиенту тем же сообщением и добавляет порядковый номер сообщения от этого клиента.
Протокол сообщения - UDP.

При закрытии клиент отправляет сообщение `close`, после этого сервер сбрасывает счетчик клиента.

## Сборка

```bash
make
```

## Запуск

Raw сокеты требуют root-прав.

```bash
make run-server
make run-client
```

Можно указать порт клиента первым аргументом:

```bash
./bin/client 7776
./bin/client 7775
```

## Пример работы

Клиент 1:

```text
$ ./bin/client 7776
Enter message: WAAAAAAAGH
Server: WAAAAAAAGH 1
Enter message: ping
Server: ping 2
Enter message: WAAAAAAAGH
Server: WAAAAAAAGH 3
Enter message: exit
```

Клиент 2:

```text
$ ./bin/client 7775
Enter message: ping
Server: ping 1
Enter message: exit
```

Сервер:

```text
$ ./bin/server
Raw echo server started on port 7777
127.0.0.1:7776 -> WAAAAAAAGH 1
127.0.0.1:7776 -> ping 2
127.0.0.1:7776 -> WAAAAAAAGH 3
Client 127.0.0.1:7776 closed
127.0.0.1:7775 -> ping 1
Client 127.0.0.1:7775 closed
```
