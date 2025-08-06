#include "InvertedIndex.h"
#include "ThreadPool.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <future>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <cctype>

bool is_valid_word(const std::string& word) {
    if (word.empty() || word.size() > 100) {
        return false;
    }
    for (char c : word) {
        if (!std::isalpha(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

void InvertedIndex::updateDocumentBaseFromStrings(const std::vector<std::string>& docs_input) {
    documents = docs_input;
    freq_dictionary.clear();

    for (size_t i = 0; i < documents.size(); ++i) {
        auto index = BuildIndexForDocument(documents[i], i);
        for (auto& [word, entries] : index) {
            freq_dictionary[word].insert(freq_dictionary[word].end(), entries.begin(), entries.end());
        }
    }
}

void InvertedIndex::updateDocumentBase(const std::vector<std::string>& file_paths) {
    documents.clear();
    freq_dictionary.clear();
    documents.resize(file_paths.size());

    std::vector<std::unordered_map<std::string, std::vector<Entry>>> partial_indices(file_paths.size());

    ThreadPool pool(std::thread::hardware_concurrency());
    std::vector<std::future<void>> futures;

    for (size_t i = 0; i < file_paths.size(); ++i) {
        futures.emplace_back(pool.enqueue([this, &file_paths, i, &partial_indices]() {
            std::ifstream file(file_paths[i]);
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << file_paths[i] << "\n";
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            documents[i] = buffer.str();

            partial_indices[i] = BuildIndexForDocument(documents[i], i);
        }));
    }

    for (auto& f : futures) f.get();

    std::unordered_set<std::string> all_words;
    for (const auto& partial : partial_indices) {
        for (const auto& [word, _] : partial) {
            all_words.insert(word);
        }
    }

    std::mutex dict_mutex;
    std::vector<std::future<void>> merge_futures;

    for (const auto& word : all_words) {
        merge_futures.emplace_back(pool.enqueue([&partial_indices, &word, this, &dict_mutex]() {
            std::vector<Entry> combined_entries;
            for (const auto& partial : partial_indices) {
                auto it = partial.find(word);
                if (it != partial.end()) {
                    combined_entries.insert(combined_entries.end(), it->second.begin(), it->second.end());
                }
            }

            std::lock_guard<std::mutex> lock(dict_mutex);
            freq_dictionary[word] = std::move(combined_entries);
        }));
    }

    for (auto& f : merge_futures) f.get();
}

std::vector<Entry> InvertedIndex::getWordCount(const std::string& word) const {
    auto it = freq_dictionary.find(word);
    return (it != freq_dictionary.end()) ? it->second : std::vector<Entry>{};
}

std::unordered_map<std::string, std::vector<Entry>>
InvertedIndex::BuildIndexForDocument(const std::string& document, size_t doc_id) {
    std::unordered_map<std::string, size_t> word_count;
    std::stringstream ss(document);
    std::string word;

    while (ss >> word) {
        // Очистка слова от пунктуации
        word.erase(std::remove_if(word.begin(), word.end(),
                    [](char c) { return !std::isalnum(static_cast<unsigned char>(c)); }),
                    word.end());

        if (is_valid_word(word)) {
            word_count[word]++;
        }
    }

    std::unordered_map<std::string, std::vector<Entry>> result;
    for (const auto& [word, count] : word_count) {
        result[word].push_back({doc_id, count});
    }
    return result;
}
