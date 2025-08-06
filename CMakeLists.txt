cmake_minimum_required(VERSION 3.20)
project(search_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Пути
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)
set(EXT_DIR ${CMAKE_SOURCE_DIR}/external)
set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/config)

# Все исходники из src (кроме main.cpp)
file(GLOB_RECURSE ALL_SOURCES "${SRC_DIR}/*.cpp")
list(FILTER ALL_SOURCES EXCLUDE REGEX "main.cpp$")

# Заголовочные директории для основного проекта
include_directories(${INC_DIR} ${EXT_DIR})

# Основной исполняемый файл с main.cpp
add_executable(search_engine
        ${ALL_SOURCES}
        "${SRC_DIR}/main.cpp"
)

target_include_directories(search_engine PRIVATE ${INC_DIR} ${EXT_DIR})

set(EXT_DIR ${CMAKE_SOURCE_DIR}/external)
include_directories(${EXT_DIR})


# Тесты
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

file(GLOB_RECURSE TEST_SOURCES "${TEST_DIR}/*.cpp")

add_executable(search_engine_tests
        ${ALL_SOURCES}  # без main.cpp из src, есть в ALL_SOURCES
        ${TEST_SOURCES}
)

target_include_directories(search_engine_tests PRIVATE ${INC_DIR} ${EXT_DIR})

target_link_libraries(search_engine_tests PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(search_engine_tests)


# Копирование ресурсов
file(COPY
        ${CMAKE_SOURCE_DIR}/resources
        ${CONFIG_DIR}
        DESTINATION ${CMAKE_BINARY_DIR}
)

