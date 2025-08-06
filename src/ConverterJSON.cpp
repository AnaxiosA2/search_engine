#include "ConverterJSON.h"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdio>

using json = nlohmann::json;
namespace fs = std::filesystem;

bool ConverterJSON::LoadConfig(const std::string& filename, std::string& error) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        error = "Config file not found: " + filename;
        return false;
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        error = std::string("Config file parse error: ") + e.what();
        return false;
    }

    if (!j.contains("config") || !j["config"].is_object()) {
        error = "Config file missing 'config' object";
        return false;
    }

    if (!j.contains("files") || !j["files"].is_array()) {
        error = "Config missing 'files' array";
        return false;
    }

    try {
        auto& cfg = j["config"];
        if (!cfg.contains("version") || !cfg["version"].is_string()) {
            error = "Config 'version' field missing or invalid";
            return false;
        }
        config_version_ = cfg["version"].get<std::string>();

        if (cfg.contains("max_responses") && cfg["max_responses"].is_number_integer()) {
            max_responses_ = cfg["max_responses"].get<int>();
        } else {
            max_responses_ = 5; // default
        }

        text_documents_.clear();
        fs::path config_path = filename;
        fs::path config_dir = config_path.parent_path();

        for (auto& file_name_json : j["files"]) {
            if (!file_name_json.is_string()) continue;

            fs::path relative_doc_path = file_name_json.get<std::string>();

            fs::path doc_path = relative_doc_path.is_absolute() ? relative_doc_path : (config_dir / relative_doc_path);
            doc_path = doc_path.lexically_normal(); // Убирает лишние ../ и ./ из пути

            std::ifstream doc_file(doc_path);
            if (!doc_file.is_open()) {
                error = "Failed to open document file: " + doc_path.string();
                return false;
            }

            std::stringstream buffer;
            buffer << doc_file.rdbuf();
            std::string content = buffer.str();

            if (content.empty()) {
                error = "Empty document file: " + doc_path.string();
                return false;
            }

            text_documents_.push_back(content);
        }
    } catch (const std::exception& e) {
        error = std::string("Config file structure error: ") + e.what();
        return false;
    }

    return true;
}


bool ConverterJSON::LoadRequests(const std::string& filename, std::string& error) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        error = "Requests file not found: " + filename;
        return false;
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        error = std::string("Requests file parse error: ") + e.what();
        return false;
    }

    if (!j.contains("requests") || !j["requests"].is_array()) {
        error = "Requests file missing 'requests' array";
        return false;
    }

    requests_.clear();
    for (auto& req : j["requests"]) {
        if (req.is_string()) {
            requests_.push_back(req.get<std::string>());
        }
    }

    return true;
}

const std::vector<std::string>& ConverterJSON::GetTextDocuments() const {
    return text_documents_;
}

const std::vector<std::string>& ConverterJSON::GetRequests() const {
    return requests_;
}

int ConverterJSON::GetResponsesLimit() const {
    return max_responses_;
}

bool ConverterJSON::CheckConfigVersion(const std::string& app_version) const {
    return config_version_ == app_version;
}

bool ConverterJSON::SaveAnswers(const std::string& filename,
                               const std::vector<std::string>& requests,
                               const std::vector<std::vector<RelativeIndex>>& answers) const {
    json output_json;
    json answers_array = json::array();

    for (size_t i = 0; i < requests.size(); ++i) {
        json answer_item;
        answer_item["request"] = requests[i];
        if (i >= answers.size() || answers[i].empty()) {
            answer_item["result"] = false;
        } else {
            answer_item["result"] = true;
            json relevance_array = json::array();
            for (const auto& rel : answers[i]) {
                relevance_array.push_back({
                    {"doc_id", rel.doc_id},
                    {"rank", rel.rank}
                });
            }
            answer_item["relevance"] = relevance_array;
        }
        answers_array.push_back(answer_item);
    }

    output_json["answers"] = answers_array;

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::remove(filename.c_str());
        return false;
    }

    file << output_json.dump(4);
    return true;
}

bool ConverterJSON::putAnswers(const std::vector<std::string>& requests,
                               const std::vector<std::vector<RelativeIndex>>& answers) const {
    return SaveAnswers("answers.json", requests, answers);
}

bool ConverterJSON::SaveRequests(const std::string& filename) const {
    nlohmann::json json_requests;
    json_requests["requests"] = nlohmann::json::array();

    for (const auto& req : requests_) {
        json_requests["requests"].push_back(req);
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    file << json_requests.dump(4);
    return true;
}


void ConverterJSON::SetRequests(const std::vector<std::string>& requests) {
    requests_ = requests;
}
