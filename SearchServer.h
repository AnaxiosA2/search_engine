#pragma once
#include "InvertedIndex.h"
#include "RelativeIndex.h"
#include <vector>
#include <string>
#include "external/json.hpp"
#include "ThreadPool.h"

struct SearchResult {
    std::vector<RelativeIndex> results;
    size_t total_count;
};

class SearchServer {
public:
    explicit SearchServer(InvertedIndex& idx);

    std::vector<SearchResult> search(const std::vector<std::string>& queries);

    void saveAnswers(
        const std::string& filename,
        const std::vector<SearchResult>& answers) const;

private:
    InvertedIndex& index;
    ThreadPool pool;

    static std::string normalizeWord(const std::string& word);
};
