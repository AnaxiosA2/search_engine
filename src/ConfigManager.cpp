#include "ConfigManager.h"
#include "json.hpp"
#include <fstream>
#include <exception>

using json = nlohmann::json;

const std::string APP_VERSION = "1.0";

std::optional<ConfigData> ConfigManager::LoadConfig(const std::string& filename, std::string& error) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        error = "config file is missing";
        return std::nullopt;
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        error = std::string("config file is invalid JSON: ") + e.what();
        return std::nullopt;
    }

    if (!j.contains("config")) {
        error = "config file is empty";
        return std::nullopt;
    }

    ConfigData data;
    try {
        auto& cfg = j["config"];
        data.config.name = cfg.at("name").get<std::string>();
        data.config.version = cfg.at("version").get<std::string>();
        if (cfg.contains("max_responses")) {
            data.config.max_responses = cfg.at("max_responses").get<int>();
        } else {
            data.config.max_responses = 5;
        }
        if (!CheckVersion(data.config.version, APP_VERSION)) {
            error = "config.json has incorrect file version";
            return std::nullopt;
        }

        if (!j.contains("files") || !j["files"].is_array()) {
            error = "config file missing or invalid 'files' array";
            return std::nullopt;
        }
        data.files = j.at("files").get<std::vector<std::string>>();
    } catch (const std::exception& e) {
        error = std::string("config file has invalid structure: ") + e.what();
        return std::nullopt;
    }

    return data;
}

bool ConfigManager::CheckVersion(const std::string& config_version, const std::string& app_version) {
    return config_version == app_version;
}

bool ConfigManager::SaveConfig(const std::string& filename, const ConfigData& config) {
    json j;
    j["config"] = {
        {"name", config.config.name},
        {"version", config.config.version},
        {"max_responses", config.config.max_responses}
    };
    j["files"] = config.files;

    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << j.dump(4);
    return true;
}
