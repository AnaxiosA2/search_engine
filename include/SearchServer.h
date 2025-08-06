#pragma once
#include <string>
#include <vector>
#include "RelativeIndex.h"
#include "json.hpp"
#include "InvertedIndex.h"

class SearchServer {
public:
    explicit SearchServer(InvertedIndex& idx, int max_responses = 5)
        : _index(idx), _max_responses(max_responses) {}

    // Поиск по запросам
    std::vector<std::vector<RelativeIndex>> search(const std::vector<std::string>& queries_input);

    // Сохранение результатов в JSON
    void saveAnswers(const std::string& filename,
                 const std::vector<std::string>& queries,
                 const std::vector<std::vector<RelativeIndex>>& results) const;

    // Установка максимального числа ответов (если нужно изменить после создания)
    void setMaxResponses(int max_responses);

private:
    InvertedIndex& _index;
    int _max_responses;
};
