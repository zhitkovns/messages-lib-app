cmake_minimum_required(VERSION 3.10)
project(journal_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Библиотека журналирования
add_library(journal_lib STATIC
    journal_lib.cpp
    journal_lib.hpp
)
target_include_directories(journal_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})  
target_link_libraries(journal_lib PRIVATE pthread)

# Основное приложение
add_executable(journal_app
    journal_app.cpp
)
target_link_libraries(journal_app PRIVATE journal_lib)  

# Приложение для статистики
add_executable(stats_collector
    stats_collector.cpp
)
target_link_libraries(stats_collector PRIVATE pthread journal_lib) 

# Тестирование
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    message(STATUS "Configuring tests...")
    enable_testing()
    
    # Тесты для журналирования
    add_executable(journal_tests 
        tests/journal_tests.cpp
    )
    target_link_libraries(journal_tests PRIVATE journal_lib pthread)
    add_test(NAME journal_tests COMMAND journal_tests)
    
    # Тесты для статистики
    add_executable(stats_tests
        tests/stats_tests.cpp
    )
    target_link_libraries(stats_tests PRIVATE pthread journal_lib)
    add_test(NAME stats_tests COMMAND stats_tests)
endif()