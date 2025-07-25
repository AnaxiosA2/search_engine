#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <future>
#include "InvertedIndex.h"
#include "SearchServer.h"
#include "ConfigUtils.h"

int main() {
    std::cout << "Welcome to Simple Search Engine!\n\n";

    std::cout << "Choose mode:\n";
    std::cout << "1) Use existing config.json\n";
    std::cout << "2) Enter data manually and save to config.json\n";
    std::cout << "Enter choice: ";

    int choice = 0;
    std::cin >> choice;
    std::wcin.ignore();

    std::vector<std::wstring> file_paths_w;
    std::vector<std::wstring> queries_w;

    if (choice == 1) {
        if (!load_config("config.json", file_paths_w, queries_w)) {
            std::cout << "Failed to load config.json. Exiting.\n";
            return 1;
        }
    } else if (choice == 2) {
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

        if (!save_config("config.json", file_paths_w, queries_w)) {
            std::wcerr << L"Failed to save config.json\n";
            return 1;
        }
        std::wcout << L"Data saved to config.json\n";
    } else {
        std::cout << "Invalid choice. Exiting.\n";
        return 1;
    }

    if (file_paths_w.empty()) {
        std::cout << "No files provided. Exiting.\n";
        return 0;
    }
    if (queries_w.empty()) {
        std::cout << "No search queries provided. Exiting.\n";
        return 0;
    }

    std::vector<std::string> file_paths_utf8;
    for (const auto& wpath : file_paths_w) {
        file_paths_utf8.push_back(wstring_to_utf8(wpath));
    }

    InvertedIndex index;

    std::cout << "Starting document indexing...\n";
    auto indexing_future = std::async(std::launch::async, [&]() {
        index.updateDocumentBase(file_paths_utf8);
    });
    indexing_future.get();
    std::cout << "Indexing completed.\n";

    SearchServer server(index);

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

    for (size_t i = 0; i < queries_utf8.size(); ++i) {
        const auto& query = queries_utf8[i];
        const auto& results = all_results[i].results;
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

    server.saveAnswers("answers.json", all_results);
    std::cout << "Search results saved to answers.json\n";

    std::cout << "Thank you for using Simple Search Engine. Goodbye!\n";
    return 0;
}
