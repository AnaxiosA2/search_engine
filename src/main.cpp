#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <future>

#include "InvertedIndex.h"
#include "SearchServer.h"
#include "ConfigUtils.h"
#include "ConverterJSON.h"

#ifdef RUN_TESTS
#include "gtest/gtest.h"
#endif

int main(int argc, char* argv[]) {
    // Запуск тестов при аргументе --test
    if (argc > 1 && std::string(argv[1]) == "--test") {
#ifdef RUN_TESTS
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
#else
        std::cerr << "Tests not compiled in this build.\n";
        return 1;
#endif
    }

    std::cout << "Welcome to Simple Search Engine!\n\n";

    std::cout << "Choose mode:\n";
    std::cout << "1) Use existing config.json and requests.json\n";
    std::cout << "2) Enter data manually and save to config.json and requests.json\n";
    std::cout << "Enter choice: ";

    int choice = 0;
    std::cin >> choice;
    std::wcin.ignore();

    std::vector<std::wstring> file_paths_w;
    std::vector<std::wstring> queries_w;

    ConverterJSON conv;
    std::string error;

    if (choice == 1) {
        // Загрузка config.json (с текстами документов)
        if (!conv.LoadConfig("config.json", error)) {
            std::cout << "Failed to load config.json: " << error << "\n";
            return 1;
        }

        // Проверка версии конфига
        if (!conv.CheckConfigVersion("1.0")) {
            std::cout << "Config file version mismatch. Expected version 1.0.\n";
            return 1;
        }

        // Загрузка запросов из requests.json
        if (!conv.LoadRequests("requests.json", error)) {
            std::cout << "Failed to load requests.json: " << error << "\n";
            return 1;
        }

        // Получаем запросы
        for (const auto& q : conv.GetRequests()) {
            queries_w.push_back(utf8_to_wstring(q));
        }
    }
    else if (choice == 2) {
        std::wcout << L"Enter file paths (one per line, empty line to finish):\n";
        while (true) {
            std::wcout << L"File path: ";
            std::wstring path;
            std::getline(std::wcin, path);
            if (path.empty()) break;

            path = normalize_dash(path);

            if (!std::filesystem::exists(path)) {
                std::wcout << L"File not found: " << path << L"\nPlease check the path and try again.\n";
                continue;
            }
            file_paths_w.push_back(path);
            std::wcout << L"Added file: " << path << L"\n";
        }

        std::wcout << L"\nEnter search queries (one per line, empty line to finish):\n";
        while (true) {
            std::wcout << L"Query: ";
            std::wstring query;
            std::getline(std::wcin, query);
            if (query.empty()) break;
            queries_w.push_back(query);
            std::wcout << L"Added query: " << query << L"\n";
        }

        // Сохраняем config.json (пути к файлам)
        if (!save_config("config.json", file_paths_w, {})) {
            std::wcerr << L"Failed to save config.json\n";
            return 1;
        }

        // Сохраняем requests.json с поисковыми запросами
        std::vector<std::string> queries_utf8;
        for (const auto& q : queries_w) queries_utf8.push_back(wstring_to_utf8(q));
        conv.SetRequests(queries_utf8);
        if (!conv.SaveRequests("requests.json")) {
            std::wcerr << L"Failed to save requests.json\n";
            return 1;
        }

        std::wcout << L"Data saved to config.json and requests.json\n";
    }
    else {
        std::cout << "Invalid choice. Exiting.\n";
        return 1;
    }

    // Проверки на пустые данные
    if (conv.GetTextDocuments().empty()) {
        std::cout << "No documents loaded. Exiting.\n";
        return 0;
    }
    if (queries_w.empty()) {
        std::cout << "No search queries provided. Exiting.\n";
        return 0;
    }

    // Индексация по содержимому документов (из конфига)
    InvertedIndex index;
    std::cout << "Starting document indexing...\n";
    auto indexing_future = std::async(std::launch::async, [&]() {
        index.updateDocumentBaseFromStrings(conv.GetTextDocuments());
    });
    indexing_future.get();
    std::cout << "Indexing completed.\n";

    SearchServer server(index, conv.GetResponsesLimit());

    // Поиск запросов (конвертируем в UTF-8)
    std::vector<std::string> queries_utf8;
    for (const auto& wquery : queries_w) {
        queries_utf8.push_back(wstring_to_utf8(wquery));
    }

    std::cout << "Starting search for queries...\n";
    auto search_future = std::async(std::launch::async, [&]() {
        return server.search(queries_utf8);
    });
    auto all_results = search_future.get();
    std::cout << "Search completed.\n";

    // Вывод результатов
    for (size_t i = 0; i < queries_utf8.size(); ++i) {
        const auto& query = queries_utf8[i];
        const auto& results = all_results[i];
        std::cout << "Query: " << query << "\nResults:\n";
        if (results.empty()) {
            std::cout << "  No results found.\n";
        } else {
            for (const auto& entry : results) {
                std::cout << "  Document #" << entry.doc_id << " - relevance: " << entry.rank << "\n";
            }
        }
        std::cout << "\n";
    }

    server.saveAnswers("answers.json", queries_utf8, all_results);

    std::cout << "Search results saved to answers.json\n";
    std::cout << "Thank you for using Simple Search Engine. Goodbye!\n";
    return 0;
}
