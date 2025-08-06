cmake_minimum_required(VERSION 3.20)

project(search_engine_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Пути к исходникам и include
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${ROOT_DIR}/include)
set(EXT_DIR ${ROOT_DIR}/external)

# Подключаем GoogleTest через FetchContent
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Сбор исходников проекта из tests/src (без main.cpp)
file(GLOB_RECURSE PROJECT_SOURCES
        "${SRC_DIR}/*.cpp"
)
list(FILTER PROJECT_SOURCES EXCLUDE REGEX "main.cpp$")

# Сбор тестовых исходников из tests (или tests/src, если там тоже есть тесты)
file(GLOB TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Создаем исполняемый файл тестов
add_executable(search_engine_tests
        ${PROJECT_SOURCES}
        ${TEST_SOURCES}
)

# Добавляем include-директории
target_include_directories(search_engine_tests PRIVATE
        ${INC_DIR}
        ${EXT_DIR}
        ${googletest_SOURCE_DIR}/googletest/include       # обязательно для gtest
)

# Линкуем с gtest_main
target_link_libraries(search_engine_tests PRIVATE gtest_main)

# Автоматическое обнаружение тестов GoogleTest
include(GoogleTest)
gtest_discover_tests(search_engine_tests)
