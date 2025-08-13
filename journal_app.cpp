#include "journal_lib.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <memory>


// Потокобезопасная очередь задач
class LogQueue {
public:
    struct Task {
        string message;
        importances importance;
    };

    void push(Task task) {
        lock_guard<mutex> lock(m_mutex);
        m_queue.push(move(task));
        m_condition.notify_one();
    }

    bool pop(Task& task) {
        unique_lock<mutex> lock(m_mutex);
        m_condition.wait(lock, [this]() { return !m_queue.empty() || m_stop; });
        
        if (m_stop && m_queue.empty()) {
            return false;
        }
        
        task = move(m_queue.front());
        m_queue.pop();
        return true;
    }

    void shutdown() {
        m_stop = true;
        m_condition.notify_all();
    }

private:
    queue<Task> m_queue;
    mutex m_mutex;
    condition_variable m_condition;
    atomic<bool> m_stop{false};
};

// Базовый класс для логгеров
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const string& message, importances importance) = 0;
};

// Только файловый логгер
class FileLogger : public ILogger {
public:
    FileLogger(const string& filename, importances default_level)
        : logger(filename, default_level) {}

    void log(const string& message, importances importance) override {
        logger.message_log(message, importance);
    }

private:
    Journal_logger logger;
};

// Комбинированный логгер (сокет + файл)
class SocketFileLogger : public ILogger {
public:
    SocketFileLogger(const string& host, int port, 
                   const string& filename, importances default_level)
        : socket_logger(host, port, default_level),
          file_logger(filename, default_level) {}

    void log(const string& message, importances importance) override {
    try {
        socket_logger.message_log(message, importance);
    } catch (const runtime_error& e) {
        cerr << "Socket error: " << e.what() << endl;
        cerr << "Message saved to file only" << endl;
    }
    file_logger.message_log(message, importance); // Всегда пишем в файл
}

private:
    Journal_logger socket_logger;
    Journal_logger file_logger;
};

// Менеджер логгирования
class LogManager {
public:
    LogManager(unique_ptr<ILogger> logger) : m_logger(move(logger)) {}
    
    ~LogManager() {
        stop();
    }

    void start() {
        m_worker = thread(&LogManager::process_tasks, this);
    }

    void stop() {
        if (m_running) {
            m_running = false;
            m_queue.shutdown();
            if (m_worker.joinable()) {
                m_worker.join();
            }
        }
    }

    void log(const string& message, importances importance) {
        m_queue.push({message, importance});
    }

private:
    void process_tasks() {
        LogQueue::Task task;
        while (m_running) {
            if (m_queue.pop(task)) {
                m_logger->log(task.message, task.importance);
            }
        }
    }

    unique_ptr<ILogger> m_logger;
    LogQueue m_queue;
    thread m_worker;
    atomic<bool> m_running{true};
};

// Обработчик ввода
class InputHandler {
public:
    InputHandler(LogManager& logger) : m_logger(logger) {}

    void run() {
        std::string input;
        while (true) {
            // Ввод сообщения
            do {
                std::cout << "Enter message (or 'quit' to exit): ";
                std::getline(std::cin, input);
                
                if (input == "quit") {
                    return;
                }
                
                if (input.empty()) {
                    std::cout << "Error: Message cannot be empty. Please try again.\n";
                }
            } while (input.empty());
    
            // Ввод уровня важности с проверкой
            importances importance;
            while (true) {
                std::cout << "Enter importance (LOW, MEDIUM, HIGH): ";
                std::string importance_str;
                std::getline(std::cin, importance_str);
                
                // Приводим к верхнему регистру
                std::transform(importance_str.begin(), importance_str.end(),
                             importance_str.begin(), ::toupper);
                
                // Проверяем допустимые значения
                if (importance_str == "LOW") {
                    importance = importances::LOW;
                    break;
                } 
                else if (importance_str == "MEDIUM") {
                    importance = importances::MEDIUM;
                    break;
                }
                else if (importance_str == "HIGH") {
                    importance = importances::HIGH;
                    break;
                }
                else if (importance_str.empty()){
                    importance=importances::MEDIUM;
                    break;
                }
                else {
                    std::cout << "Error: Invalid importance level. "
                              << "Please enter LOW, MEDIUM or HIGH.\n";
                }
            }
            
            // Логирование сообщения
            try {
                m_logger.log(input, importance);
            } catch (const std::exception& e) {
                std::cerr << "Logging error: " << e.what() << std::endl;
            }
        }
    }

private:
    LogManager& m_logger;
};

void print_usage() {
    cout << "Usage:\n"
         << "  File mode: journal_app <filename> [default_importance]\n"
         << "  Socket mode: journal_app --socket <host> <port> <filename> [default_importance]\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    try {
        unique_ptr<ILogger> logger;
        importances default_level = importances::MEDIUM;

        // Режим сокета + файла
        if (string(argv[1]) == "--socket") {
            if (argc < 5) {
                cerr << "Error: Socket mode requires host, port and filename\n";
                return 1;
            }

            string host = argv[2];
            int port = stoi(argv[3]);
            string filename = argv[4];

            if (argc > 5) {
                string level_str = argv[5];
                if (level_str == "LOW") default_level = importances::LOW;
                else if (level_str == "HIGH") default_level = importances::HIGH;
            }

            logger = make_unique<SocketFileLogger>(host, port, filename, default_level);
        } 
        // Файловый режим
        else {
            string filename = argv[1];

            if (argc > 2) {
                string level_str = argv[2];
                if (level_str == "LOW") default_level = importances::LOW;
                else if (level_str == "HIGH") default_level = importances::HIGH;
            }

            logger = make_unique<FileLogger>(filename, default_level);
        }

        LogManager log_manager(move(logger));
        log_manager.start();

        {
            InputHandler input(log_manager);
            input.run();
        }

        log_manager.stop();
    } 
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}