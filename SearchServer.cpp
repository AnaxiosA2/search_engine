#include "SearchServer.h"
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <unordered_set>

using json = nlohmann::json;

SearchServer::SearchServer(InvertedIndex& idx)
    : index(idx), pool(std::thread::hardware_concurrency()) {}

std::string SearchServer::normalizeWord(const std::string& word) {
    std::string result;
    result.reserve(word.size());

    for (char c : word) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            result += static_cast<char>(std::tolower(c));
        }
    }

    return result;
}

std::vector<SearchResult> SearchServer::search(const std::vector<std::string>& queries) {
    std::vector<std::future<SearchResult>> futures;
    futures.reserve(queries.size());

    for (const auto& query : queries) {
        futures.push_back(pool.enqueue([this, query]() {
            std::istringstream stream(query);
            std::string word;
            std::unordered_set<std::string> unique_words;

            while (stream >> word) {
                std::string normalized = normalizeWord(word);
                if (!normalized.empty()) {
                    unique_words.insert(normalized);
                }
            }

            if (unique_words.empty()) {
                return SearchResult{{}, 0};
            }

            struct WordFreq {
                std::string word;
                size_t total_count;
            };
            std::vector<WordFreq> words_freq;
            for (const auto& w : unique_words) {
                size_t freq = 0;
                auto entries = index.getWordCount(w);
                for (const auto& e : entries) {
                    freq += e.count;
                }
                words_freq.push_back({w, freq});
            }

            std::sort(words_freq.begin(), words_freq.end(), [](const WordFreq& a, const WordFreq& b) {
                return a.total_count < b.total_count;
            });

            std::unordered_map<size_t, size_t> doc_freq_map;
            if (words_freq.empty()) {
                return SearchResult{{}, 0};
            }

            const auto& first_word = words_freq[0].word;
            auto first_entries = index.getWordCount(first_word);
            for (const auto& e : first_entries) {
                doc_freq_map[e.doc_id] = e.count;
            }

            for (size_t i = 1; i < words_freq.size(); ++i) {
                std::unordered_map<size_t, size_t> new_map;
                auto entries = index.getWordCount(words_freq[i].word);
                std::unordered_map<size_t, size_t> word_docs;
                for (const auto& e : entries) {
                    word_docs[e.doc_id] = e.count;
                }

                for (const auto& [doc_id, freq] : doc_freq_map) {
                    if (word_docs.find(doc_id) != word_docs.end()) {
                        new_map[doc_id] = freq + word_docs[doc_id];
                    }
                }
                doc_freq_map = std::move(new_map);

                if (doc_freq_map.empty()) {
                    return SearchResult{{}, 0};
                }
            }

            size_t max_count = 0;
            for (const auto& [_, freq] : doc_freq_map) {
                if (freq > max_count) {
                    max_count = freq;
                }
            }

            std::vector<RelativeIndex> results;
            results.reserve(doc_freq_map.size());
            for (const auto& [doc_id, freq] : doc_freq_map) {
                float rank = max_count ? static_cast<float>(freq) / max_count : 0.f;
                results.push_back({doc_id, rank});
            }

            std::sort(results.begin(), results.end(), [](const RelativeIndex& a, const RelativeIndex& b) {
                if (a.rank == b.rank)
                    return a.doc_id < b.doc_id;
                return b.rank < a.rank;
            });

            return SearchResult{results, results.size()};
        }));
    }

    std::vector<SearchResult> all_results;
    all_results.reserve(queries.size());
    for (auto& f : futures) {
        all_results.push_back(f.get());
    }
    return all_results;
}

void SearchServer::saveAnswers(const std::string& filename, const std::vector<SearchResult>& answers) const {
    json output = json::array();

    for (const auto& queryResult : answers) {
        json jQuery;
        jQuery["total_count"] = queryResult.total_count;
        jQuery["results"] = json::array();
        for (const auto& [docId, rank] : queryResult.results) {
            jQuery["results"].push_back({{"doc_id", docId}, {"rank", rank}});
        }
        output.push_back(jQuery);
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file for writing: " << filename << '\n';
        return;
    }

    file << output.dump(4);
}
