#include "journal_lib.hpp"
#include <stdexcept>
#include <iomanip>
#include <sstream>

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
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif
    connect();
}

SocketOutput::~SocketOutput() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

void SocketOutput::write(const std::string& message) {
    if (sockfd == INVALID_SOCKET) {
        try {
            connect();
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Reconnect failed: " + std::string(e.what()));
        }
    }

    int result = send(sockfd, message.c_str(), message.size(), 0);
    if (result == SOCKET_ERROR) {
        disconnect();
        throw std::runtime_error("Socket send failed (WSAGetLastError: " + std::to_string(WSAGetLastError()) + ")");
    }
}

bool SocketOutput::is_connected() const {
#ifdef _WIN32
    return sockfd != INVALID_SOCKET;
#else
    return sockfd != -1;
#endif
}

void SocketOutput::connect() {
#ifdef _WIN32
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        throw std::runtime_error("Socket creation failed");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);

    if (::connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
        throw std::runtime_error("Connection failed");
    }
#else
    // Linux/Unix реализация
#endif
}

void SocketOutput::disconnect() {
#ifdef _WIN32
    if (sockfd != INVALID_SOCKET) {
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
    }
#else
    // Linux/Unix реализация
#endif
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
    localtime_s(&time_info, &timestamp);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &time_info);

    const char* importance_str = "";
    switch (importance) {
        case importances::LOW:    importance_str = "LOW";    break;
        case importances::MEDIUM: importance_str = "MEDIUM"; break;
        case importances::HIGH:   importance_str = "HIGH";   break;
    }

    return "[" + std::string(time_buf) + "] [" + importance_str + "] " + message;
}

importances parse_importance(const std::string& msg) {
    if (msg.find("[LOW]") != std::string::npos) return importances::LOW;
    if (msg.find("[HIGH]") != std::string::npos) return importances::HIGH;
    return importances::MEDIUM;
}