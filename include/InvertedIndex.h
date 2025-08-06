#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Entry.h"
#include <mutex>

class InvertedIndex {
public:
    void updateDocumentBase(const std::vector<std::string>& file_paths);
    std::vector<Entry> getWordCount(const std::string& word) const;
    void updateDocumentBaseFromStrings(const std::vector<std::string>& docs_input);

private:
    std::mutex index_mutex;
    std::unordered_map<std::string, std::vector<Entry>> BuildIndexForDocument(const std::string& document, size_t doc_id);
    std::vector<std::string> documents;
    std::unordered_map<std::string, std::vector<Entry>> freq_dictionary;
};
