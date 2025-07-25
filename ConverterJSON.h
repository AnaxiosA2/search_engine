#pragma once

#include <string>
#include <vector>

class ConverterJSON {
public:
    std::vector<std::string> LoadDocuments(const std::string& filename);
    std::vector<std::string> LoadRequests(const std::string& filename);
};
