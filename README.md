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
**Целевой ОС является Linux** 

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

3. Если Вы хотите поменять уровень важности сообщений по умолчанию, это можно сделать в файле `.vscode/tasks.json`. После внесенных изменений обязательно следует пересобрать проект.

##### 💻 Запуск через терминал
1. Перейдите в директорию проекта, а затем в папку build (создайте, если не её нет):
   ```
   mkdir -p build && cd build
   ```
2. Соберите проект:
   ```
   cmake .. && make
   ```
3. Режимы (**):
   
   3.1. Файловый:
   ```
   ./journal_app log.txt MEDIUM
   ```

   3.2. Сетевой (в разных терминалах):
   ```
   # Терминал 1 - сервер статистики
   ./stats_collector 8080 10 60

   # Терминал 2 - клиент с сокетами
   ./journal_app --socket 127.0.0.1 8080 log.txt MEDIUM
   ```
(**) - Вы можете указать нужный Вам файл для журнала или он создатся автоматически при первом запуске. При указании уровня важности сообщений есть возможность выбрать один из трёх: LOW, MEDIUM, HIGH.

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

Here's the English translation of your documentation:

### 📌 Message Logging with Different Priority Levels

#### 🔹 Project Objective
Develop a library for logging messages with different priority levels and an application demonstrating the library's functionality.

#### 🔹 Tasks
**Part 1:**
Develop a library for logging text messages. Use a text file as the log.

Library requirements:
1) The library should have two build options: dynamic/static
2) During initialization, the library should accept:
   - Log filename
   - Default message priority level (messages below this level won't be logged)
   - Priority levels should use an enum with clear names (3 levels sufficient)
3) Log entries must contain:
   - Message text
   - Priority level
   - Timestamp
4) Allow changing the default priority level after initialization
5) (*):
   - Implement socket logging
   - Socket logging interface should match file logging interface

**Part 2:**
Develop a console multithreaded application to test the logging library.

Application requirements:
1) The application must:
   - Use the library from Part 1
   - Accept user input (message + optional priority level)
   - Pass data to a separate thread for logging (thread-safe implementation)
   - Wait for new user input after processing
2) Command-line parameters: log filename and default priority level
3) Internal logic can be custom implementation

**Part 3 (*):**
Develop a console program to collect statistics from socket data (from Part 1.5 logging).

Application requirements:
1) The program must:
   - Receive log data via socket
   - Display received messages
   - Calculate message statistics:
     i. Total messages
     ii. Messages by priority level
     iii. Messages in last hour
   - Calculate message length statistics:
     i. Minimum
     ii. Maximum
     iii. Average
   - Display statistics:
     i. After every N messages
     ii. After T seconds timeout (if statistics changed)
2) Command-line parameters: socket connection details, N and T values

(*) - Optional but included in project

#### 🔹 Project Architecture
```
.
├── journal_lib.hpp       # Logging library
├── journal_lib.cpp       # Library implementation
├── journal_app.cpp       # Client application
├── stats_collector.cpp   # Statistics collector
└── tests/          
    ├── journal_tests.cpp # Logging tests
    └── stats_tests.cpp   # Statistics tests
```
**Target OS: Linux** 

#### 🔹 How to Work with the Project

##### 🖥️ VS Code Launch
Preconfigured tasks (see `.vscode/tasks.json`):
1. **File mode** - logs to file only:
   - Ctrl + Shift + P => >Tasks: Run Task => `run-journal-file`
   - Default parameters: `log.txt MEDIUM`

2. **Network mode** - logs to file and network:
   - First launch `run-stats-collector` (stats server), then `run-journal-socket` (socket client)
   - Or use `run-full-program` for combined launch

3. To change default priority level, modify `.vscode/tasks.json` and rebuild.

##### 💻 Terminal Launch
1. Navigate to project directory:
   ```
   mkdir -p build && cd build
   ```
2. Build project:
   ```
   cmake .. && make
   ```
3. Launch modes (**):
   
   3.1. File mode:
   ```
   ./journal_app log.txt MEDIUM
   ```

   3.2. Network mode (separate terminals):
   ```
   # Terminal 1 - stats server
   ./stats_collector 8080 10 60

   # Terminal 2 - socket client
   ./journal_app --socket 127.0.0.1 8080 log.txt MEDIUM
   ```
(**) - You can specify custom log filename (will auto-create). Priority levels: LOW, MEDIUM, HIGH.

#### 🔹 Key Actions
- Enter messages in console
- Specify priority level (LOW, MEDIUM, HIGH)
- Type `quit` to exit

#### 🔹 Testing
1. Navigate to project directory:
   ```
   mkdir -p build && cd build
   ```
2. Build with tests:
   ```
   cmake -DBUILD_TESTS=ON .. && make -j4
   ```
3. Run tests:
   
   3.1. Logging tests:
   ```
   ./journal_tests
   ```
   3.2. Statistics tests:
   ```
   ./stats_tests
   ```
4. Disable tests after completion:
   ```
   cmake -DBUILD_TESTS=OFF .. 
   ```

--- 
