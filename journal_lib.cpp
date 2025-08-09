#include "journal_lib.hpp"
#include <stdexcept>
#include <iomanip>
#include <sstream>

Journal_logger::Journal_logger(const std::string& filename, importances default_importance)
    : journal_filename(filename), default_importance(default_importance) {
    open_journal_file();
}

Journal_logger::~Journal_logger() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

void Journal_logger::message_log(const std::string& message, importances importance) {
    if (importance < default_importance) {
        return;  // Не записываем, если уровень ниже установленного
    }

    log_entry entry = create_log_entry(message, importance);
    log_file << format_log_entry(entry) << std::endl;
}

void Journal_logger::message_log(const std::string& message) {
    message_log(message, default_importance);
}

void Journal_logger::set_default_importance(importances new_importance) {
    default_importance = new_importance;
}

log_entry Journal_logger::create_log_entry(const std::string& message, importances importance) const {
    return {
        message,
        importance,
        std::time(nullptr)  // Текущее время
    };
}

std::string Journal_logger::format_log_entry(const log_entry& entry) const {
    std::stringstream ss;
    char time_buffer[80];
    std::tm time_info;

    localtime_s(&time_info, &entry.receipt_time);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &time_info);

    ss << "[" << time_buffer << "] "
       << "[";
    
    switch (entry.message_importance) {
        case importances::LOW:    ss << "LOW";    break;
        case importances::MEDIUM: ss << "MEDIUM"; break;
        case importances::HIGH:   ss << "HIGH";   break;
    }
    
    ss << "] " << entry.message;
    
    return ss.str();
}

void Journal_logger::open_journal_file() {
    log_file.open(journal_filename, std::ios::app);
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to open log file: " + journal_filename);
    }
}