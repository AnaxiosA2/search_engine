#include "ConfigManager.h"
#include "external/json.hpp"
#include <fstream>

using json = nlohmann::json;

bool saveConfig(const std::string& filename, const ConfigData& config) {
    json j;
    j["files"] = config.files;
    j["requests"] = config.requests;

    std::ofstream file(filename);
    if (!file.is_open())
        return false;

    file << j.dump(4);
    return true;
}

bool loadConfig(const std::string& filename, ConfigData& config) {
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    json j;
    file >> j;

    if (j.contains("files") && j["files"].is_array()) {
        config.files = j["files"].get<std::vector<std::string>>();
    } else {
        return false;
    }

    if (j.contains("requests") && j["requests"].is_array()) {
        config.requests = j["requests"].get<std::vector<std::string>>();
    } else {
        return false;
    }

    return true;
}
