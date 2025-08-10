#include "journal_lib.hpp"
#include <iostream>
#include <map>
#include <string>

int main() {
    // Получаем ввод от пользователя
    std::string message, importance_str;
    std::cout << "Enter a message: ";
    std::getline(std::cin, message);
    
    std::cout << "Enter the importance level (LOW, MEDIUM, HIGH): ";
    std::getline(std::cin, importance_str);

    // Преобразуем строку в enum
    importances importance = importances::MEDIUM; // Значение по умолчанию
    
    if (importance_str == "LOW") {
        importance = importances::LOW;
    }
    else if (importance_str == "MEDIUM") {
        importance = importances::MEDIUM;
    }
    else if (importance_str == "HIGH") {
        importance = importances::HIGH;
    }
    else {
        std::cout << "Empty. Using MEDIUM by default." << std::endl;
    }

    // Здесь создаем логгер и записываем сообщение
    Journal_logger logger("log.txt");
    logger.message_log(message, importance);

    return 0;
}