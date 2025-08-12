#include "journal_lib.hpp"
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <unistd.h>

//=== FileOutput ===//
FileOutput::FileOutput(const std::string& filename) 
    : filename(filename) {
    reopen();
}

FileOutput::~FileOutput() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

void FileOutput::write(const std::string& message) {
    if (!log_file.is_open()) {
        reopen();
    }
    log_file << message << std::endl;
}

bool FileOutput::is_connected() const {
    return log_file.is_open();
}

void FileOutput::reopen() {
    log_file.open(filename, std::ios::app);
    if (!log_file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
}

//=== SocketOutput ===//
SocketOutput::SocketOutput(const std::string& host, int port) 
    : host(host), port(port) {
    connect();
}

SocketOutput::~SocketOutput() {
    disconnect();
}

void SocketOutput::write(const std::string& message) {
    if (sockfd == -1) {
        try {
            connect();
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Reconnect failed: " + std::string(e.what()));
        }
    }

    int result = send(sockfd, message.c_str(), message.size(), 0);
    if (result == -1) {
        disconnect();
        throw std::runtime_error("Socket send failed: " + std::string(strerror(errno)));
    }
}

bool SocketOutput::is_connected() const {
    return sockfd != -1;
}

void SocketOutput::connect() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);

    if (::connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
        close(sockfd);
        sockfd = -1;
        throw std::runtime_error("Connection failed: " + std::string(strerror(errno)));
    }
}

void SocketOutput::disconnect() {
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
}

//=== Journal_logger ===//
Journal_logger::Journal_logger(const std::string& filename, importances importance)
    : output(std::make_unique<FileOutput>(filename)),
      default_importance(importance) {}

Journal_logger::Journal_logger(const std::string& host, int port, importances importance)
    : output(std::make_unique<SocketOutput>(host, port)),
      default_importance(importance) {}

void Journal_logger::message_log(const std::string& message, importances importance) {
    if (importance < default_importance) return;
    if (message.empty() || message[0]=='\n') return;

    time_t now = time(nullptr);
    std::string formatted = format_log(message, importance, now);
    output->write(formatted);
}

std::string Journal_logger::format_log(
    const std::string& message, 
    importances importance, 
    time_t timestamp
) const {
    char time_buf[64];
    tm time_info;
    localtime_r(&timestamp, &time_info);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &time_info);

    const char* importance_str = "";
    switch (importance) {
        case importances::LOW:    importance_str = "LOW";    break;
        case importances::MEDIUM: importance_str = "MEDIUM"; break;
        case importances::HIGH:   importance_str = "HIGH";   break;
    }

    return "[" + std::string(time_buf) + "] [" + importance_str + "] " + message;
}