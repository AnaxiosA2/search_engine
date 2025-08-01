cmake_minimum_required(VERSION 3.20)
project(search_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Пути
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)
set(EXT_DIR ${CMAKE_SOURCE_DIR}/external)

# Собираем все исходники из src (включая main.cpp)
file(GLOB_RECURSE ALL_SOURCES "${SRC_DIR}/*.cpp")

# Для тестов исключаем main.cpp (чтобы не было конфликта точек входа)
list(FILTER ALL_SOURCES EXCLUDE REGEX "main.cpp$")

# Заголовочные директории
include_directories(${INC_DIR} ${EXT_DIR})

# Основной исполняемый файл с main.cpp
add_executable(search_engine
        ${ALL_SOURCES}
        "${SRC_DIR}/main.cpp"
)

# Подключаем GoogleTest
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

# Все тестовые исходники
file(GLOB_RECURSE TEST_SOURCES "${TEST_DIR}/*.cpp")

# Создаём исполняемый файл для тестов (без main.cpp из src)
add_executable(search_engine_tests
        ${ALL_SOURCES}  # без main.cpp
        ${TEST_SOURCES}
)

target_link_libraries(search_engine_tests PRIVATE gtest_main)

target_include_directories(search_engine_tests PRIVATE ${INC_DIR} ${EXT_DIR})

include(GoogleTest)
gtest_discover_tests(search_engine_tests)

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})
