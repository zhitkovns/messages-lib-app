cmake_minimum_required(VERSION 3.10)
project(JournalApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Библиотека (отдельная цель)
add_library(journal_lib STATIC 
    journal_lib.cpp
    journal_lib.hpp
)
target_include_directories(journal_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 2. Исполняемый файл (тестовое приложение - отдельная цель)
add_executable(journal_app journal_app.cpp)
target_link_libraries(journal_app PRIVATE journal_lib)

# Автоматическое копирование .exe в папку build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})