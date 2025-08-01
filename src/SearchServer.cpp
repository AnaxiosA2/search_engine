#include "SearchServer.h"
#include <algorithm>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include "json.hpp"

std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string>& queries_input) {
    std::vector<std::vector<RelativeIndex>> results;

    for (const auto& query : queries_input) {
        // 1. Разбиваем запрос на слова
        std::istringstream iss(query);
        std::vector<std::string> words;
        std::string word;
        while (iss >> word) {
            words.push_back(word);
        }

        // 2. Получаем уникальные слова
        std::unordered_set<std::string> unique_words(words.begin(), words.end());

        // 3. Сортируем по частоте в базе (по возрастанию)
        std::vector<std::string> sorted_words(unique_words.begin(), unique_words.end());
        std::sort(sorted_words.begin(), sorted_words.end(), [this](const std::string& a, const std::string& b) {
            return _index.getWordCount(a).size() < _index.getWordCount(b).size();
        });

        // 4. Получаем документы по первому слову
        std::vector<Entry> first_word_entries = _index.getWordCount(sorted_words.front());
        if (first_word_entries.empty()) {
            results.emplace_back(); // пустой результат
            continue;
        }

        // 5. Инициализируем карту релевантности
        std::unordered_map<size_t, int> doc_relevance;
        for (auto& e : first_word_entries) {
            doc_relevance[e.doc_id] = e.count;
        }

        // 6. Обрабатываем остальные слова
        for (size_t i = 1; i < sorted_words.size(); ++i) {
            auto entries = _index.getWordCount(sorted_words[i]);
            std::unordered_map<size_t, int> current_counts;
            for (auto& e : entries) {
                current_counts[e.doc_id] = e.count;
            }

            for (auto it = doc_relevance.begin(); it != doc_relevance.end();) {
                auto found = current_counts.find(it->first);
                if (found == current_counts.end()) {
                    it = doc_relevance.erase(it);
                } else {
                    it->second += found->second;
                    ++it;
                }
            }
        }

        if (doc_relevance.empty()) {
            results.emplace_back();
            continue;
        }

        // 7. Находим максимум релевантности
        int max_rel = 0;
        for (const auto& [_, rel] : doc_relevance) {
            if (rel > max_rel) max_rel = rel;
        }

        // 8. Собираем и нормализуем результаты
        std::vector<RelativeIndex> relative_indices;
        for (const auto& [doc_id, rel] : doc_relevance) {
            relative_indices.push_back({doc_id, static_cast<float>(rel) / max_rel});
        }

        std::sort(relative_indices.begin(), relative_indices.end(),
                  [](const RelativeIndex& a, const RelativeIndex& b) {
                      return a.rank > b.rank;
                  });

        // 9. Ограничиваем по max_responses
        if (relative_indices.size() > _max_responses) {
            relative_indices.resize(_max_responses);
        }

        results.push_back(std::move(relative_indices));
    }

    return results;
}

// Сохраняет результаты в JSON-файл
// Добавим параметр queries, чтобы знать текст запросов
void SearchServer::saveAnswers(
    const std::string& filename,
    const std::vector<std::string>& queries,
    const std::vector<std::vector<RelativeIndex>>& results) const
{
    nlohmann::json json_root;
    json_root["answers"] = nlohmann::json::array();

    for (size_t i = 0; i < queries.size(); ++i) {
        nlohmann::json answer;
        answer["request"] = queries[i];
        const auto& query_results = results[i];

        if (query_results.empty()) {
            answer["result"] = false;
        } else {
            answer["result"] = true;
            answer["relevance"] = nlohmann::json::array();
            for (const auto& entry : query_results) {
                answer["relevance"].push_back({
                    {"doc_id", entry.doc_id},
                    {"rank", entry.rank}
                });
            }
        }

        json_root["answers"].push_back(answer);
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    file << json_root.dump(4);
}

// Устанавливает максимальное число ответов
void SearchServer::setMaxResponses(int max_responses) {
    _max_responses = max_responses;
}
