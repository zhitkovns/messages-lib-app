#include "journal_lib.hpp"
#include <cassert>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

// Вспомогательные функции для тестирования
string read_last_line(const string& filename) {
    ifstream file(filename);
    string line, last_line;
    while (getline(file, line)) {
        if (!line.empty()) {
            last_line = line;
        }
    }
    return last_line;
}

void clear_test_file(const string& filename) {
    if (filesystem::exists(filename)) {
        filesystem::remove(filename);
    }
}

// Тест 1: Проверка создания файла лога
void test_file_creation() {
    const string test_file = "test_creation.log";
    clear_test_file(test_file);
    
    {
        Journal_logger logger(test_file, importances::LOW);
        logger.message_log("Test message", importances::LOW);
    }
    
    assert(filesystem::exists(test_file));
    clear_test_file(test_file);
}

// Тест 2: Проверка фильтрации по уровню важности
void test_importance_filter() {
    const string test_file = "test_filter.log";
    clear_test_file(test_file);
    
    {
        Journal_logger logger(test_file, importances::MEDIUM);
        logger.message_log("Low message", importances::LOW);    // Не должно записаться
        logger.message_log("High message", importances::HIGH);  // Должно записаться
    }
    
    string content = read_last_line(test_file);
    assert(content.find("High message") != string::npos);
    assert(content.find("Low message") == string::npos);
    clear_test_file(test_file);
}

// Тест 3: Проверка формата записи
void test_log_format() {
    const string test_file = "test_format.log";
    clear_test_file(test_file);
    
    {
        Journal_logger logger(test_file, importances::LOW);
        logger.message_log("Test format", importances::MEDIUM);
    }
    
    string line = read_last_line(test_file);
    assert(line.find("[MEDIUM]") != string::npos);
    assert(line.find("Test format") != string::npos);
    assert(line.find("] [") != string::npos); // Проверка формата
    clear_test_file(test_file);
}

// Тест 4: Проверка изменения уровня важности
void test_change_importance() {
    const string test_file = "test_change.log";
    clear_test_file(test_file);
    
    {
        Journal_logger logger(test_file, importances::HIGH);
        logger.set_default_importance(importances::LOW);
        logger.message_log("Now visible", importances::MEDIUM);
    }
    
    string content = read_last_line(test_file);
    assert(content.find("Now visible") != string::npos);
    clear_test_file(test_file);
}

// Тест 5: Проверка многопоточной записи (потокобезопасность)
void test_thread_safety() {
    const string test_file = "test_thread.log";
    clear_test_file(test_file);
    
    Journal_logger logger(test_file, importances::LOW);
    
    auto worker = [&logger](int id) {
        for (int i = 0; i < 10; ++i) {
            logger.message_log("Thread " + to_string(id) + " message " + to_string(i), 
                             importances::LOW);
        }
    };
    
    thread t1(worker, 1);
    thread t2(worker, 2);
    t1.join();
    t2.join();
    
    // Подсчитываем количество строк в файле
    ifstream file(test_file);
    int line_count = count(
        istreambuf_iterator<char>(file),
        istreambuf_iterator<char>(), 
        '\n'
    );
    assert(line_count == 20); // 10 сообщений от каждого потока
    clear_test_file(test_file);
}

int main() {
    try {
        cout << "Running journal library tests...\n";
        
        test_file_creation();
        test_importance_filter();
        test_log_format();
        test_change_importance();
        test_thread_safety();
        
        cout << "All tests passed successfully!\n";
        return 0;
    } catch (const exception& e) {
        cerr << "Test failed: " << e.what() << endl;
        return 1;
    }
}