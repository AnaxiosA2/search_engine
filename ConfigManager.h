#pragma once

#include <string>
#include <vector>

struct ConfigData {
    std::vector<std::string> files;
    std::vector<std::string> requests;
};

bool saveConfig(const std::string& filename, const ConfigData& config);
bool loadConfig(const std::string& filename, ConfigData& config);
