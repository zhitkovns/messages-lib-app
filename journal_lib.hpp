#include <string>
#include <ctime>
#include <fstream>

// Уровни важности
enum class importances {LOW, MEDIUM, HIGH};

// Запись журнала
typedef struct{
    std::string message;
    importances message_importance;
    time_t receipt_time;
} log_entry;

// Класс для работы с журналом
class Journal_logger{
    public:
        Journal_logger(const std::string& filename, importances default_importance = importances::MEDIUM);
        ~Journal_logger();

        // Запись сообщения в журнал (с указанием важности)
        void message_log(const std::string& message, importances importance);
    
        // Запись сообщения с уровнем по умолчанию
        void message_log(const std::string& message);

        // Установка нового уровня важности по умолчанию
        void set_default_importance(importances new_importance);

    private:
        std::string journal_filename;
        importances default_importance;
        std::ofstream log_file; // Поток для записи в файл

        // Создание записи журнала
        log_entry create_log_entry(const std::string& message, importances importance) const;
    
        // Преобразование записи в строку для файла
        std::string format_log_entry(const log_entry& entry) const;
    
        // Открытие файла журнала
        void open_journal_file();
};  