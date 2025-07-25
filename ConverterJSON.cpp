#include "ConverterJSON.h"
#include "external/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

std::vector<std::string> ConverterJSON::LoadDocuments(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file: " << filename << "\n";
        return {};
    }

    json j;
    file >> j;

    std::vector<std::string> documents;

    if (j.contains("files") && j["files"].is_array()) {
        for (const auto& doc : j["files"]) {
            if (doc.is_string()) {
                documents.push_back(doc.get<std::string>());
            }
        }
    } else {
        std::cerr << "JSON file does not contain 'files' array.\n";
    }

    return documents;
}

std::vector<std::string> ConverterJSON::LoadRequests(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file: " << filename << "\n";
        return {};
    }

    json j;
    file >> j;

    std::vector<std::string> requests;

    if (j.contains("requests") && j["requests"].is_array()) {
        for (const auto& req : j["requests"]) {
            if (req.is_string()) {
                requests.push_back(req.get<std::string>());
            }
        }
    } else {
        std::cerr << "JSON file does not contain 'requests' array.\n";
    }

    return requests;
}
