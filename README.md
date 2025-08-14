## RU

### 📌 Логгирование сообщений с разными уровнями важности

#### 🔹 Цель проекта
Разработать библиотеку для записи сообщений в журнал с разными уровнями важности и приложение, демонстрирующее работу библиотеки. 

#### 🔹 Задачи
**Часть 1:** 
Разработать библиотеку для записи текстовых сообщений в журнал. В качестве журнала использовать текстовый файл.

Требования к разрабатываемой библиотеке:

1) Библиотека должна иметь 2 варианта сборки: динамическая/статическая 
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

1) Приложение должно:
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

1) Приложение должно:
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
   - `Ctrl+Shift+P` → `>Tasks: Run Task` → `run-journal-file`
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

#### 📚 Сборка библиотеки журналирования

**Вариант A: Собрать и запустить сразу**  
```
# Переходим в папку сборки
mkdir -p build && cd build

# Собираем СТАТИЧЕСКУЮ версию (библиотека + программа)
cmake .. -DBUILD_SHARED_LIBS=OFF
make -j$(nproc)
./journal_app log.txt MEDIUM  # запуск

# ИЛИ динамическую:
cmake .. -DBUILD_SHARED_LIBS=ON
make -j$(nproc)
LD_LIBRARY_PATH=. ./journal_app log.txt MEDIUM  # запуск с подгрузкой .so
```

**Вариант B: Собрать библиотеку отдельно, потом программу**  
Если нужно переиспользовать библиотеку в других проектах:  
```
# Собираем библиотеку (например, динамическую)
cmake .. -DBUILD_SHARED_LIBS=ON
make journal_lib -j$(nproc)  # только библиотека

# Затем собираем программу, которая линкуется с ней
make journal_app -j$(nproc)
LD_LIBRARY_PATH=. ./journal_app log.txt MEDIUM
```

**❗Важно:**
После смены типа библиотеки нужно:  
   - Либо **очистить сборку** (`rm -rf build/*`),  
   - Либо **перезапустить CMake** с новым флагом.  

#### 🔹 Ключевые действия в проекте
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

#### 🔹 Примеры:

1. Запись в журнале:
   ```
   [2025-08-12 14:39:43] [HIGH] message
   ```
2. Вывод статистики:
   ```
   === Statistics ===
   Total messages: 5
   By importance:
   LOW:    0
   MEDIUM: 4
   HIGH:   1
   Last hour: 5 messages
   Message lengths:
   Min: 43
   Max: 46
   Avg: 43.8
   =================
   ```
---


## EN

### 📌 Message Logging with Different Priority Levels  

#### 🔹 Project Objective  
Develop a library for logging messages with different priority levels and an application to demonstrate its functionality.  

#### 🔹 Tasks  
**Part 1:**  
Develop a library for logging text messages to a journal (text file).  

**Library Requirements:**  
1) The library must support two build types: **static/dynamic**  
2) During initialization, the library must accept:  
   - Journal filename  
   - Default message priority level (messages below this level are ignored)  
   - Priority levels should use an enum with clear names (3 levels suffice)  
3) Journal entries must include:  
   - Message text  
   - Priority level  
   - Timestamp  
4) Allow changing the default priority level post-initialization  
5) (*):  
   - Implement socket logging  
   - Socket logging interface must match file logging  

**Part 2:**  
Develop a multithreaded console application to test the library.  

**Application Requirements:**  
1) The app must:  
   - Use the Part 1 library  
   - Accept user input (message + optional priority level)  
   - Pass data to a separate thread for logging (thread-safe)  
   - Await new input after processing  
2) Command-line args: journal filename + default priority level  
3) Internal logic is customizable  

**Part 3 (*):**  
Develop a console program to collect socket log statistics (from Part 1.5).  

**Requirements:**  
1) The program must:  
   - Receive log data via socket  
   - Display messages  
   - Calculate:  
     - Total messages  
     - Messages by priority  
     - Messages in the last hour  
     - Message length stats (min/max/avg)  
   - Display stats:  
     - After every *N* messages  
     - After *T*-second timeout (if stats changed)  
2) Args: Socket params, *N*, *T*  

(*) Optional but included in the project.  

---

#### 🔹 Project Architecture  
```
.
├── journal_lib.hpp       # Logging library  
├── journal_lib.cpp       # Library implementation  
├── journal_app.cpp       # Client app  
├── stats_collector.cpp   # Stats collector  
└── tests/          
    ├── journal_tests.cpp # Logging tests  
    └── stats_tests.cpp   # Stats tests  
```
**Target OS:** Linux  

---

#### 🔹 How to Use  

##### 🖥️ VS Code Launch  
Preconfigured tasks (see `.vscode/tasks.json`):  
1. **File mode** (logs to file only):  
   - `Ctrl+Shift+P` → `>Tasks: Run Task` → `run-journal-file`  
   - Default args: `log.txt MEDIUM`  

2. **Network mode** (file + socket logging):  
   - First launch `run-stats-collector` (stats server), then `run-journal-socket`  
   - Or use `run-full-program` for combined launch  

3. To change default priority, edit `.vscode/tasks.json` and rebuild.  

##### 💻 Terminal Launch  
1. Navigate to the project:  
   ```
   mkdir -p build && cd build  
   cmake .. && make  
   ```  
2. **File mode**:  
   ```
   ./journal_app log.txt MEDIUM  
   ```  
3. **Network mode** (separate terminals):  
   ```
   # Terminal 1 (stats server):  
   ./stats_collector 8080 10 60  

   # Terminal 2 (socket client):  
   ./journal_app --socket 127.0.0.1 8080 log.txt MEDIUM  
   ```  
   - Logfile auto-creates if missing. Priority levels: `LOW`/`MEDIUM`/`HIGH`.  

---

#### 📚 Library Build Options  

**Option A: Build & Run (All-in-One)**  
```
# Static build  
cmake .. -DBUILD_SHARED_LIBS=OFF  
make -j$(nproc)  
./journal_app log.txt MEDIUM  

# Dynamic build  
cmake .. -DBUILD_SHARED_LIBS=ON  
make -j$(nproc)  
LD_LIBRARY_PATH=. ./journal_app log.txt MEDIUM  # Load .so from current dir  
```  

**Option B: Build Library Separately**  
```
# Build dynamic library only  
cmake .. -DBUILD_SHARED_LIBS=ON  
make journal_lib -j$(nproc)  

# Build & link the app  
make journal_app  
LD_LIBRARY_PATH=. ./journal_app log.txt MEDIUM  
```  

**❗Note:**  
- After switching build types:  
  - Clean the build (`rm -rf build/*`) **or**  
  - Re-run CMake with the new flag.  

---

#### 🔹 Key Actions  
- Enter messages in the console  
- Specify priority (`LOW`/`MEDIUM`/`HIGH`)  
- Type `quit` to exit  

---

#### 🔹 Testing  
1. Build with tests:  
   ```
   cmake -DBUILD_TESTS=ON .. && make -j4  
   ```  
2. Run tests:  
   ```
   ./journal_tests      # Logging tests  
   ./stats_tests        # Stats tests  
   ```  
3. Disable tests:  
   ```
   cmake -DBUILD_TESTS=OFF ..  
   ```  

#### 🔹 Examples:

1. Journal entry:
   ```
   [2025-08-12 14:39:43] [HIGH] message
   ```
2. Statistics output:
   ```
   === Statistics ===
   Total messages: 5
   By priority:
   LOW:    0
   MEDIUM: 4
   HIGH:   1
   Last hour: 5 messages
   Message lengths:
   Min: 43
   Max: 46
   Avg: 43.8
   =================
   ```

--- 