#pragma once
#include <string>
#include <ctime>
#include <fstream>
#include <memory>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

enum class importances { LOW, MEDIUM, HIGH };
importances parse_importance(const std::string& msg);

class LogOutput {
public:
    virtual ~LogOutput() = default;
    virtual void write(const std::string& message) = 0;
    virtual bool is_connected() const = 0;
};

class FileOutput : public LogOutput {
public:
    FileOutput(const std::string& filename);
    ~FileOutput() override;
    void write(const std::string& message) override;
    bool is_connected() const override;

private:
    std::string filename;
    std::ofstream log_file;
    void reopen();
};

class SocketOutput : public LogOutput {
public:
    SocketOutput(const std::string& host, int port);
    ~SocketOutput() override;
    void write(const std::string& message) override;
    bool is_connected() const override;

private:
    std::string host;
    int port;
#ifdef _WIN32
    SOCKET sockfd = INVALID_SOCKET;
#else
    int sockfd = -1;
#endif
    void connect();
    void disconnect();
};

class Journal_logger {
public:
    Journal_logger(const std::string& filename, importances importance);
    Journal_logger(const std::string& host, int port, importances importance);
    ~Journal_logger() = default;

    void message_log(const std::string& message, importances importance);
    void message_log(const std::string& message);
    void set_default_importance(importances new_importance);

private:
    std::unique_ptr<LogOutput> output;
    importances default_importance;

    std::string format_log(
        const std::string& message, 
        importances importance, 
        time_t timestamp
    ) const;
};