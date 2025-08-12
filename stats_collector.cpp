#include "journal_lib.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

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

importances parse_importance(const string& msg) {
    if (msg.find("[LOW]") != string::npos) return importances::LOW;
    if (msg.find("[HIGH]") != string::npos) return importances::HIGH;
    return importances::MEDIUM;
}

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
    stats.by_importance[imp]++;
    
    stats.last_hour.emplace_back(now, len);
    stats.last_hour.erase(
        remove_if(stats.last_hour.begin(), stats.last_hour.end(),
            [now](const auto& entry) { return now - entry.first > 3600; }),
        stats.last_hour.end()
    );
}

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
        cout << "Usage: " << argv[0] << " <port> <N> <T>\n";
        return 1;
    }

    const int port = stoi(argv[1]);
    const size_t N = stoul(argv[2]);
    const size_t T = stoul(argv[3]);

    // Создание сокета
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == -1) {
        cerr << "Socket creation failed: " << strerror(errno) << endl;
        return 1;
    }

    // Установка опции для повторного использования адреса
    int opt = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "Setsockopt failed: " << strerror(errno) << endl;
        close(listen_socket);
        return 1;
    }

    // Привязка к порту
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(listen_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        cerr << "Bind failed: " << strerror(errno) << endl;
        close(listen_socket);
        return 1;
    }

    // Ожидание подключений
    if (listen(listen_socket, SOMAXCONN) == -1) {
        cerr << "Listen failed: " << strerror(errno) << endl;
        close(listen_socket);
        return 1;
    }

    cout << "Listening on port " << port << "..." << endl;

    // Принятие подключения
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(listen_socket, (sockaddr*)&client_addr, &client_len);
    if (client_socket == -1) {
        cerr << "Accept failed: " << strerror(errno) << endl;
        close(listen_socket);
        return 1;
    }

    cout << "Client connected from " << inet_ntoa(client_addr.sin_addr) 
         << ":" << ntohs(client_addr.sin_port) << endl;

    MessageStats stats;
    auto last_print_time = time(nullptr);
    size_t last_total = 0;

    char buffer[4096];
    while (true) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                cout << "Client disconnected" << endl;
            } else {
                cerr << "Receive error: " << strerror(errno) << endl;
            }
            break;
        }

        buffer[bytes_received] = '\0';
        string message(buffer);
        time_t now = time(nullptr);

        update_stats(stats, message, now);
        cout << "Received: " << message << endl;

        // Проверяем условия вывода статистики
        if ((stats.total % N == 0) || (difftime(now, last_print_time) >= T)) {
            print_stats(stats);
            last_print_time = now;
        }
    }

    // Завершение работы
    close(client_socket);
    close(listen_socket);
    return 0;
}