#include "journal_lib.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct MessageStats {
    size_t total = 0;
    size_t min_len = 0;
    size_t max_len = 0;
    double avg_len = 0.0;
    unordered_map<importances, size_t> by_importance = {
        {importances::LOW, 0},
        {importances::MEDIUM, 0},
        {importances::HIGH, 0}
    };
    vector<pair<time_t, size_t>> last_hour;
};

// Парсинг строки лога в уровень важности
importances parse_importance(const string& msg) {
    if (msg.find("[LOW]") != string::npos) return importances::LOW;
    if (msg.find("[HIGH]") != string::npos) return importances::HIGH;
    return importances::MEDIUM;
}

// Обновление статистики
void update_stats(MessageStats& stats, const string& msg, time_t now) {
    stats.total++;
    const size_t len = msg.size();
    
    if (stats.total == 1) {
        stats.min_len = stats.max_len = len;
    } else {
        stats.min_len = min(stats.min_len, len);
        stats.max_len = max(stats.max_len, len);
    }
    stats.avg_len = (stats.avg_len * (stats.total - 1) + len) / stats.total;
    
    importances imp = parse_importance(msg);
    stats.by_importance[imp]++;  // Гарантированно существует
    
    stats.last_hour.emplace_back(now, len);
    stats.last_hour.erase(
        remove_if(stats.last_hour.begin(), stats.last_hour.end(),
            [now](const auto& entry) { return now - entry.first > 3600; }),
        stats.last_hour.end()
    );
}

// Вывод статистики в консоль
void print_stats(const MessageStats& stats) {
    cout << "\n=== Statistics ===\n";
    cout << "Total messages: " << stats.total << "\n";
    cout << "By importance:\n"
              << "  LOW:    " << stats.by_importance.at(importances::LOW) << "\n"
              << "  MEDIUM: " << stats.by_importance.at(importances::MEDIUM) << "\n"
              << "  HIGH:   " << stats.by_importance.at(importances::HIGH) << "\n";
    cout << "Last hour: " << stats.last_hour.size() << " messages\n";
    cout << "Message lengths:\n"
              << "  Min: " << stats.min_len << "\n"
              << "  Max: " << stats.max_len << "\n"
              << "  Avg: " << stats.avg_len << "\n";
    cout << "=================\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <port> <N> <T>\n";
        return 1;
    }

    const int port = stoi(argv[1]);
    const size_t N = stoul(argv[2]);
    const size_t T = stoul(argv[3]);

    // Инициализация Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed\n";
        return 1;
    }

    // Создание сокета
    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    // Привязка к порту
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(listen_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Bind failed\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    // Ожидание подключений
    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    cout << "Listening on port " << port << "...\n";

    // Принятие подключения
    SOCKET client_socket = accept(listen_socket, nullptr, nullptr);
    if (client_socket == INVALID_SOCKET) {
        cerr << "Accept failed\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    MessageStats stats;
    auto last_print_time = time(nullptr);
    size_t last_total = 0;

    char buffer[4096];
    while (int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) {
        if (bytes_received <= 0) break;

        buffer[bytes_received] = '\0';
        std::string message(buffer);
        time_t now = time(nullptr);

        update_stats(stats, message, now);
        std::cout << "Received: " << message << "\n";

        // Проверяем условия вывода статистики
        if ((stats.total % N == 0) || (difftime(now, last_print_time) >= T)) {
            print_stats(stats);
            last_print_time = now;
        }
    }

    // Завершение работы
    closesocket(client_socket);
    closesocket(listen_socket);
    WSACleanup();
    return 0;
}