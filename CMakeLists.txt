cmake_minimum_required(VERSION 3.20)
project(search_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(search_engine
        main.cpp
        InvertedIndex.cpp
        SearchServer.cpp
        ConverterJSON.cpp
        ConfigManager.cpp
        ConfigUtils.cpp
)

target_include_directories(search_engine PRIVATE ${CMAKE_SOURCE_DIR})
