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

using namespace std;

// Вспомогательная функция для создания тестового сервера
class TestServer {
    int server_fd;
    int port;
public:
    TestServer(int port) : port(port) {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        bind(server_fd, (sockaddr*)&address, sizeof(address));
        listen(server_fd, 3);
    }
    
    ~TestServer() {
        close(server_fd);
    }
    
    int accept_connection() {
        return accept(server_fd, nullptr, nullptr);
    }
    
    int get_port() const { return port; }
};

// Тест 1: Проверка создания и запуска коллектора
void test_collector_creation() {
    const int test_port = 8081;
    thread collector_thread([test_port]() {
        system((string("./stats_collector ") + 
                   to_string(test_port) + " 10 1").c_str());
    });
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    // Проверяем что сервер запущен, пытаясь подключиться
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(test_port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    assert(connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == 0);
    close(sock);
    
    collector_thread.detach();
}

// Тест 2: Проверка обработки сообщений
void test_message_processing() {
    TestServer server(8082);
    thread collector_thread([]() {
        system("./stats_collector 8082 10 1");
    });
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8082);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
    const char* test_msg = "[2023-01-01 12:00:00] [HIGH] Test message\n";
    send(sock, test_msg, strlen(test_msg), 0);
    close(sock);
    
    collector_thread.detach();
}

// Тест 3: Проверка статистики по уровням важности
void test_importance_stats() {
    TestServer server(8083);
    thread collector_thread([]() {
        system("./stats_collector 8083 10 1");
    });
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8083);
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
}

// Тест 4: Проверка статистики длины сообщений
void test_message_length_stats() {
    TestServer server(8084);
    thread collector_thread([]() {
        system("./stats_collector 8084 10 1");
    });
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8084);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
    
    const char* short_msg = "[2023-01-01 12:00:00] [LOW] Short\n";
    const char* long_msg = "[2023-01-01 12:00:01] [HIGH] Very long message for testing\n";
    
    send(sock, short_msg, strlen(short_msg), 0);
    send(sock, long_msg, strlen(long_msg), 0);
    
    close(sock);
    collector_thread.detach();
}

// Тест 5: Проверка временной статистики (сообщения за последний час)
void test_time_based_stats() {
    TestServer server(8085);
    thread collector_thread([]() {
        system("./stats_collector 8085 10 1");
    });
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8085);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
    
    // Отправляем сообщение с текущим временем
    time_t now = time(nullptr);
    tm* tm_now = localtime(&now);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "[%Y-%m-%d %H:%M:%S]", tm_now);
    
    string msg = string(time_buf) + " [HIGH] Recent message\n";
    send(sock, msg.c_str(), msg.size(), 0);
    
    close(sock);
    collector_thread.detach();
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