#include "journal_lib.hpp"
#include <cassert>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <atomic>
#include <memory>

using namespace std;

// Глобальный счетчик для уникальных портов
static atomic<int> port_counter(9000); // Начинаем с порта 9000

// Функция для получения свободного порта
int get_free_port() {
    while (true) {
        int port = port_counter++;
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        
        if (bind(sock, (sockaddr*)&addr, sizeof(addr))) {
            close(sock);
            continue;
        }
        
        close(sock);
        return port;
    }
}

// Тихая версия system() без вывода
int quiet_system(const char* cmd) {
    int ret = system((string(cmd) + " > /dev/null 2>&1").c_str());
    this_thread::sleep_for(chrono::milliseconds(100));
    return ret;
}

// Тест 1: Проверка создания и запуска коллектора
void test_collector_creation() {
    int port = get_free_port();
    thread collector_thread([port]() {
        quiet_system((string("./stats_collector ") + to_string(port) + " 10 1").c_str());
    });
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    assert(connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == 0);
    close(sock);
    
    collector_thread.detach();
    this_thread::sleep_for(chrono::milliseconds(500));
}

// Тест 2: Проверка обработки сообщений
void test_message_processing() {
    int port = get_free_port();
    thread collector_thread([port]() {
        quiet_system((string("./stats_collector ") + to_string(port) + " 10 1").c_str());
    });
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
    const char* test_msg = "[2023-01-01 12:00:00] [HIGH] Test message\n";
    send(sock, test_msg, strlen(test_msg), 0);
    close(sock);
    
    collector_thread.detach();
    this_thread::sleep_for(chrono::milliseconds(500));
}

// Тест 3: Проверка статистики по уровням важности
void test_importance_stats() {
    int port = get_free_port();
    thread collector_thread([port]() {
        quiet_system((string("./stats_collector ") + to_string(port) + " 10 1").c_str());
    });
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
    
    const char* messages[] = {
        "[2023-01-01 12:00:00] [LOW] Message 1\n",
        "[2023-01-01 12:00:01] [HIGH] Message 2\n",
        "[2023-01-01 12:00:02] [MEDIUM] Message 3\n"
    };
    
    for (const auto* msg : messages) {
        send(sock, msg, strlen(msg), 0);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    close(sock);
    collector_thread.detach();
    this_thread::sleep_for(chrono::milliseconds(500));
}

// Тест 4: Проверка статистики длины сообщений
void test_message_length_stats() {
    int port = get_free_port();
    thread collector_thread([port]() {
        quiet_system((string("./stats_collector ") + to_string(port) + " 10 1").c_str());
    });
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
    
    const char* short_msg = "[2023-01-01 12:00:00] [LOW] Short\n";
    const char* long_msg = "[2023-01-01 12:00:01] [HIGH] Very long message for testing\n";
    
    send(sock, short_msg, strlen(short_msg), 0);
    send(sock, long_msg, strlen(long_msg), 0);
    
    close(sock);
    collector_thread.detach();
    this_thread::sleep_for(chrono::milliseconds(500));
}

// Тест 5: Проверка временной статистики
void test_time_based_stats() {
    int port = get_free_port();
    thread collector_thread([port]() {
        quiet_system((string("./stats_collector ") + to_string(port) + " 10 1").c_str());
    });
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
    
    time_t now = time(nullptr);
    tm* tm_now = localtime(&now);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "[%Y-%m-%d %H:%M:%S]", tm_now);
    
    string msg = string(time_buf) + " [HIGH] Recent message\n";
    send(sock, msg.c_str(), msg.size(), 0);
    
    close(sock);
    collector_thread.detach();
    this_thread::sleep_for(chrono::milliseconds(500));
}

int main() {
    cout << "Running stats_collector tests...\n";
    
    test_collector_creation();
    test_message_processing();
    test_importance_stats();
    test_message_length_stats();
    test_time_based_stats();
    
    cout << "All stats_collector tests completed!\n";
    return 0;
}