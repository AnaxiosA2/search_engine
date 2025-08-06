#include "gtest/gtest.h"
#include "ConfigManager.h"
#include <fstream>
#include <filesystem>

using namespace std;

const string TEST_CONFIG_VALID = R"(
{
  "config": {
    "name": "TestEngine",
    "version": "1.0",
    "max_responses": 7
  },
  "files": [
    "file1.txt",
    "file2.txt"
  ]
}
)";

const string TEST_CONFIG_INVALID_JSON = "{ bad json ";

const string TEST_CONFIG_WRONG_VERSION = R"(
{
  "config": {
    "name": "TestEngine",
    "version": "9.9",
    "max_responses": 5
  },
  "files": []
}
)";

const string TEST_CONFIG_EMPTY = R"({})";

class ConfigManagerTest : public ::testing::Test {
protected:
    void WriteToFile(const string& filename, const string& content) {
        ofstream ofs(filename);
        ofs << content;
    }

    void RemoveFile(const string& filename) {
        std::filesystem::remove(filename);
    }
};

TEST_F(ConfigManagerTest, LoadValidConfig) {
    const string filename = "test_config.json";
    WriteToFile(filename, TEST_CONFIG_VALID);

    string error;
    auto config_opt = ConfigManager::LoadConfig(filename, error);

    EXPECT_TRUE(config_opt.has_value());
    EXPECT_EQ(error, "");
    EXPECT_EQ(config_opt->config.name, "TestEngine");
    EXPECT_EQ(config_opt->config.version, "1.0");
    EXPECT_EQ(config_opt->config.max_responses, 7);
    EXPECT_EQ(config_opt->files.size(), 2);

    RemoveFile(filename);
}

TEST_F(ConfigManagerTest, MissingFile) {
    string error;
    auto config_opt = ConfigManager::LoadConfig("nonexistent.json", error);

    EXPECT_FALSE(config_opt.has_value());
    EXPECT_EQ(error, "config file is missing");
}

TEST_F(ConfigManagerTest, InvalidJson) {
    const string filename = "invalid_json.json";
    WriteToFile(filename, TEST_CONFIG_INVALID_JSON);

    string error;
    auto config_opt = ConfigManager::LoadConfig(filename, error);

    EXPECT_FALSE(config_opt.has_value());
    // Проверяем, что ошибка содержит "config file is invalid JSON"
    EXPECT_NE(error.find("config file is invalid JSON"), std::string::npos);

    RemoveFile(filename);
}

TEST_F(ConfigManagerTest, EmptyConfig) {
    const string filename = "empty_config.json";
    WriteToFile(filename, TEST_CONFIG_EMPTY);

    string error;
    auto config_opt = ConfigManager::LoadConfig(filename, error);

    EXPECT_FALSE(config_opt.has_value());
    EXPECT_EQ(error, "config file is empty");

    RemoveFile(filename);
}

TEST_F(ConfigManagerTest, WrongVersion) {
    const string filename = "wrong_version.json";
    WriteToFile(filename, TEST_CONFIG_WRONG_VERSION);

    string error;
    auto config_opt = ConfigManager::LoadConfig(filename, error);

    EXPECT_FALSE(config_opt.has_value());
    EXPECT_EQ(error, "config.json has incorrect file version");

    RemoveFile(filename);
}
