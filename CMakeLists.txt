cmake_minimum_required(VERSION 3.10)
project(journal_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# --- Библиотека для ведения журнала ---
add_library(journal_lib STATIC
    journal_lib.cpp
    journal_lib.hpp
)

target_link_libraries(journal_lib PRIVATE pthread)

# --- Приложение для журнала ---
add_executable(journal_app
    journal_app.cpp
)

target_link_libraries(journal_app PRIVATE journal_lib)

# --- Приложение для сбора статистики ---
add_executable(stats_collector
    stats_collector.cpp
)

target_link_libraries(stats_collector PRIVATE pthread)