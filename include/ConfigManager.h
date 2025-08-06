#pragma once
#include <string>
#include <vector>
#include <optional>

struct ConfigData {
    struct ConfigInfo {
        std::string name;
        std::string version;
        int max_responses = 5;
    } config;
    std::vector<std::string> files;
};

class ConfigManager {
public:
    // Загружает конфиг
    static std::optional<ConfigData> LoadConfig(const std::string& filename, std::string& error);

    // Проверяет версию конфига
    static bool CheckVersion(const std::string& config_version, const std::string& app_version);

    // Сохраняет config
    static bool SaveConfig(const std::string& filename, const ConfigData& config);
};
