#include "journal_lib.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <functional>

// Потокобезопасная очередь задач
class LogQueue {
public:
    struct Task {
        std::string message;
        importances importance;
    };

    void push(Task task) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(task));
        m_condition.notify_one();
    }

    bool pop(Task& task) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this]() { return !m_queue.empty() || m_stop; });
        
        if (m_stop && m_queue.empty()) {
            return false;
        }
        
        task = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    void shutdown() {
        m_stop = true;
        m_condition.notify_all();
    }

private:
    std::queue<Task> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::atomic<bool> m_stop{false};
};

// Менеджер логгирования
class LogManager {
public:
    LogManager(const std::string& filename, importances default_level)
        : m_logger(filename, default_level) {}
    
    ~LogManager() {
        stop();
    }

    void start() {
        m_worker = std::thread(&LogManager::process_tasks, this);
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

    void log(const std::string& message, importances importance) {
        m_queue.push({message, importance});
    }

private:
    void process_tasks() {
        LogQueue::Task task;
        while (m_running) {
            if (m_queue.pop(task)) {
                m_logger.message_log(task.message, task.importance);
            }
        }
    }

    Journal_logger m_logger;
    LogQueue m_queue;
    std::thread m_worker;
    std::atomic<bool> m_running{true};
};

// Обработчик пользовательского ввода
class InputHandler {
public:
    InputHandler(LogManager& logger) : m_logger(logger) {}

    void run() {
        std::string input;
        while (true) {
            std::cout << "Enter message (or 'quit' to exit): ";
            std::getline(std::cin, input);
            
            if (input == "quit") {
                break;
            }

            std::cout << "Enter importance (LOW, MEDIUM, HIGH): ";
            std::string importance_str;
            std::getline(std::cin, importance_str);

            importances importance = parse_importance(importance_str);
            m_logger.log(input, importance);
        }
    }

private:
    importances parse_importance(const std::string& str) {
        if (str == "LOW") return importances::LOW;
        if (str == "HIGH") return importances::HIGH;
        return importances::MEDIUM; // default
    }

    LogManager& m_logger;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <logfile> [default_importance]\n";
        return 1;
    }

    importances default_level = importances::MEDIUM;
    if (argc > 2) {
        std::string level_str = argv[2];
        if (level_str == "LOW") default_level = importances::LOW;
        else if (level_str == "HIGH") default_level = importances::HIGH;
    }

    LogManager logger(argv[1], default_level);
    logger.start();

    {
        InputHandler input(logger);
        input.run();
    }

    logger.stop();
    return 0;
}