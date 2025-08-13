## RU

### 📌 Логгирование сообщений с разными уровнями важности

#### 🔹 Цель проекта
Разработать библиотеку для записи сообщений в журнал с разными уровнями важности и приложение, демонстрирующее работу библиотеки. 

#### 🔹 Задачи
**Часть 1:** 
Разработать библиотеку для записи текстовых сообщений в журнал. В качестве журнала использовать текстовый файл.

Требования к разрабатываемой библиотеке:
­1) Библиотека должна иметь 2 варианта сборки: динамическая/статическая
2) Библиотека при инициализации должна принимать следующие параметры:
   - Имя файла журнала
   - Уровень важности сообщения по умолчанию. Сообщения с уровнем ниже заданного не должны
     записываться в журнал. Уровень рекомендуется задавать с помощью перечисления с понятными
     именами. Достаточно трех уровней важности
3) В журнале должны быть сохранена следующая информация:
   - Текст сообщения
   - Уровень важности
   - Время получения сообщения
4) После инициализации должна быть возможность менять уровень важности сообщений по умолчанию
5) (*)
   - Добавить реализацию с записью лога в сокет
   - Интерфейс логирования через сокет не должен отличаться от интерфейса логирования в файл

**Часть 2:**
Разработать консольное многопоточное приложение для проверки библиотеки записи сообщений в журнал.

Требования к приложению:
­1) Приложение должно:
   - Подключать и использовать библиотеку, реализованную в Части 1, для записи сообщений в журнал
   - В консоли принимать сообщение и уровень важности этого сообщения от пользователя. Уровень
     важности может отсутствовать
   - Передавать принятые данные от пользователя в отдельный поток для записи в журнал. Передача данных
     должна быть потокобезопасной
   - Ожидать нового ввода от пользователя после передачи данных
2) Параметрами приложения должны быть имя файла журнала и уровень важности сообщения по умолчанию
3) Внутреннюю логику приложения придумать самостоятельно

**Часть 3 (*):**
Реализовать консольную программу для сбора статистик по данным из сокета (от библиотеки логирования из части 1.5).

Требования к приложению:
­1) Приложение должно:
   - Принимать из сокета от библиотеки логирования данные
   - Выводить в консоль принятое сообщение лога
   - Выполнять подсчет статистик кол-ва сообщений:
        i.    сообщений всего
        ii.   сообщений по уровню важности
        iii.  сообщений за последний час
   - Выполнять подсчет статистик длин сообщений:
        i.    Минимум
        ii.   Максимум
        iii.  Средняя
   - Выводить в консоль собранную статистику:
        i.    после приема N-го сообщения
        ii.   после таймаута T секунд, при условии, что статистика изменилась с момента последней выдачи
2) Параметрами приложения должны быть параметры подключения для прослушивания сокета, значения N и T

(*) - Дополнительно, но также включено в проект

#### 🔹 Архитектура проекта
```
.
├── journal_lib.hpp       # Библиотека журналирования
├── journal_lib.cpp       # Реализация библиотеки журналирования
├── journal_app.cpp       # Клиентское приложение
├── stats_collector.cpp   # Консольная программа для сбора статистики
└── tests/          
    ├── journal_tests.cpp # Тестирование журналирования
    └── stats_tests.cpp   # Тестирование программы для сбора статистики
```

#### 🔹 Как работать с проектом

##### 🖥️ Запуск через VS Code
В проекте настроены задачи для VS Code (см. `.vscode/tasks.json`):
1. **Файловый режим** - логи пишутся только в файл:
   - Ctrl + Shift + P => >Tasks: Run Task => `run-journal-file`
   - Параметры по умолчанию: `log.txt MEDIUM`

2. **Сетевой режим** - логи пишутся и в файл, и отправляются по сети:
   - Запуск аналогично п. 1
   - Сначала запустите `run-stats-collector` (сервер статистики), `run-journal-socket` (клиент с сокетами)
   - Или `run-full-program` для одновременного запуска

##### 💻 Запуск через терминал
1. Перейдите в директорию проекта, а затем в папку build (создайте, если не её нет):
   ```
   mkdir -p build && cd build
   ```
2. Соберите проект:
   ```
   cmake .. && make
   ```

3.1. Файловый режим:
   ```
   ./journal_app log.txt MEDIUM
   ```

3.2. Сетевой режим (в разных терминалах):
   ```
   # Терминал 1 - сервер статистики
   ./stats_collector 8080 10 60

   # Терминал 2 - клиент с сокетами
   ./journal_app --socket 127.0.0.1 8080 log.txt MEDIUM
   ```

#### 🔹 Ключевые действия
- Вводите сообщения в консоль
- Указывайте уровень важности (LOW, MEDIUM, HIGH)
- Для выхода вводите `quit`

#### 🔹 Как тестировать
1. Перейдите в директорию проекта, а затем в папку build (создайте, если не её нет):
   ```
   mkdir -p build && cd build
   ```
2. Соберите проект с флагом для тестов:
   ```
   cmake -DBUILD_TESTS=ON .. && make -j4
   ```
3. Запустите тестирование:
   3.1. Библиотеки и приложения журналирования:
   ```
   ./journal_tests
   ```
   3.2. Программы для сбора статистики:
   ```
   ./stats_tests
   ```
4. После завершения тестирования измените флаг тестирования:
   ```
   cmake -DBUILD_TESTS=OFF .. 
   ```

---


## EN

### 📌 Message Logging with Different Importance Levels

#### 🔹 Project Goal
Develop a library for logging messages with different importance levels and an application to demonstrate the library's functionality.

#### 🔹 Tasks
**Part 1:**
Develop a library for logging text messages to a journal (text file).

Library requirements:
1) The library should support both static and dynamic builds
2) During initialization, the library should accept:
   - Journal filename
   - Default message importance level (messages below this level won't be logged)
   - Importance levels should use clear enum names (three levels sufficient)
3) Journal entries must contain:
   - Message text
   - Importance level
   - Timestamp
4) Must allow changing default importance level after initialization
5) (*)
   - Add socket logging implementation
   - Socket logging interface should be identical to file logging interface

**Part 2:**
Develop a console multithreaded application to test the logging library.

Application requirements:
1) The application must:
   - Use the library from Part 1 for message logging
   - Accept messages and importance levels from user input (level optional)
   - Pass user data to a separate thread for logging (thread-safe implementation)
   - Wait for new user input after processing
2) Application parameters:
   - Journal filename
   - Default message importance level
3) Internal application logic to be designed independently

**Part 3 (*):**
Develop a console program to collect statistics from socket data (from Part 1.5 logging library).

Requirements:
1) The program must:
   - Receive log data through sockets
   - Display received log messages in console
   - Calculate message statistics:
        i. Total messages
        ii. Messages by importance level
        iii. Messages from last hour
   - Calculate message length statistics:
        i. Minimum length
        ii. Maximum length
        iii. Average length
   - Display statistics:
        i. After every N messages
        ii. After timeout T seconds (if statistics changed since last output)
2) Program parameters:
   - Socket connection parameters
   - N (message count)
   - T (timeout in seconds)

(*) - Additional but included in the project

#### 🔹 Project Architecture
```
.
├── journal_lib.hpp       # Logging library interface
├── journal_lib.cpp       # Logging library implementation
├── journal_app.cpp       # Client application
├── stats_collector.cpp   # Statistics collection program
└── tests/          
    ├── journal_tests.cpp # Logging tests
    └── stats_tests.cpp   # Statistics program tests
```

#### 🔹 How to Work with the Project

##### 🖥️ Running via VS Code
Preconfigured tasks in `.vscode/tasks.json`:
1. **File mode** - logs only to file:
   - Task: `run-journal-file`
   - Parameters: `log.txt MEDIUM`

2. **Network mode** - logs to both file and socket:
   - First run `run-stats-collector` (stats server)
   - Then `run-journal-socket` (socket client)
   - Or `run-full-program` to run both simultaneously

##### 💻 Running via Terminal
1. Build the project:
```bash
mkdir -p build && cd build
cmake .. && make
```

2. File mode:
```bash
./journal_app log.txt MEDIUM
```

3. Network mode (in separate terminals):
```bash
# Terminal 1 - stats server
./stats_collector 8080 10 60

# Terminal 2 - socket client
./journal_app --socket 127.0.0.1 8080 log.txt MEDIUM
```

#### 🔹 Key Actions
- Enter messages in console
- Specify importance level (LOW, MEDIUM, HIGH)
- Type `quit` to exit

--- 
