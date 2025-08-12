cmake_minimum_required(VERSION 3.10)
project(journal_app)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    set(WS2_LIB ws2_32)
endif()


# --- Библиотека для ведения журнала ---
add_library(journal_lib STATIC
    journal_lib.cpp
    journal_lib.hpp
)

target_link_libraries(journal_lib PRIVATE ${WS2_LIB})


# --- Приложение для журнала ---
add_executable(journal_app
    journal_app.cpp
)

target_link_libraries(journal_app PRIVATE journal_lib ${WS2_LIB})


# --- Приложение для сбора статистики ---
add_executable(stats_collector
    stats_collector.cpp
)

target_link_libraries(stats_collector PRIVATE ${WS2_LIB})