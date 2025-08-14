#pragma once
#include <string>
#include <ctime>
#include <fstream>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


enum class importances { LOW, MEDIUM, HIGH }; // Уровни важности сообщений

// Базовый интерфейс для вывода логов
class LogOutput {
public:
    virtual ~LogOutput() = default;
    virtual void write(const std::string& message) = 0; // Запись сообщения
    virtual bool is_connected() const = 0; // Проверка подключения
};

// Реализация вывода в файл
class FileOutput : public LogOutput {
public:
    FileOutput(const std::string& filename);
    ~FileOutput() override;
    void write(const std::string& message) override;
    bool is_connected() const override;

private:
    std::string filename;
    std::ofstream log_file;
    void reopen(); // Переоткрытие файла при ошибках
};

// Реализация вывода через сокет
class SocketOutput : public LogOutput {
public:
    SocketOutput(const std::string& host, int port);
    ~SocketOutput() override;
    void write(const std::string& message) override;
    bool is_connected() const override;

private:
    std::string host;
    int port;
    int sockfd = -1;   // Дескриптор сокета
    void connect();    // Установка соединения
    void disconnect(); // Разрыв соединения
};

// Основной класс логирования
class Journal_logger {
public:
    // Конструктор для файлового режима
    Journal_logger(const std::string& filename, importances importance); 
    // Конструктор для сокетного режима
    Journal_logger(const std::string& host, int port, importances importance); 
    
    ~Journal_logger() = default;

    // Запрет копирования и присваивания
    Journal_logger(const Journal_logger&) = delete;
    Journal_logger& operator=(const Journal_logger&) = delete;

    void message_log(const std::string& message, importances importance);
    void set_default_importance(importances new_importance);
    importances get_default_importance() const {
        return default_importance;
    }

private:
    std::unique_ptr<LogOutput> output;
    importances default_importance;

    // Форматирование записи лога
    std::string format_log(
        const std::string& message, 
        importances importance, 
        time_t timestamp
    ) const;
};