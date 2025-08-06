// tests/TestConverterJSON.cpp
#include "gtest/gtest.h"
#include "ConverterJSON.h"
#include <string>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

const std::string config_dir = "../tests/config/";
const std::string resources_dir = "../tests/resources/";

TEST(ConverterJSONTest, LoadValidConfig) {
    std::string error;
    ConverterJSON conv;

    const std::string config_path = config_dir + "test_config.json";
    const std::string requests_path = config_dir + "test_requests.json";

    bool loaded = conv.LoadConfig(config_path, error);
    EXPECT_TRUE(loaded) << "LoadConfig failed: " << error;

    auto docs = conv.GetTextDocuments();
    ASSERT_FALSE(docs.empty());
    ASSERT_EQ(docs.size(), 2);
    std::ifstream file1(resources_dir + "doc1.txt");
    std::string expected_doc1((std::istreambuf_iterator<char>(file1)), std::istreambuf_iterator<char>());
    EXPECT_EQ(docs[0], expected_doc1);


    bool requests_loaded = conv.LoadRequests(requests_path, error);
    EXPECT_TRUE(requests_loaded) << "LoadRequests failed: " << error;

    auto requests = conv.GetRequests();
    ASSERT_EQ(requests.size(), 2);
    EXPECT_EQ(requests[0], "apple");
}

TEST(ConverterJSONTest, LoadInvalidConfig) {
    std::string error;
    ConverterJSON conv;
    const std::string invalid_path = config_dir + "bad_config.json";
    bool loaded = conv.LoadConfig(invalid_path, error);
    EXPECT_FALSE(loaded);
}

TEST(ConverterJSONTest, GetResponsesLimitDefaultAndCustom) {
    std::string error;

    {
        ConverterJSON conv;
        const std::string path = config_dir + "test_config_limit.json";
        ASSERT_TRUE(conv.LoadConfig(path, error)) << error;
        EXPECT_EQ(conv.GetResponsesLimit(), 5);
    }

    {
        ConverterJSON conv;
        const std::string path = config_dir + "config_with_max.json";
        ASSERT_TRUE(conv.LoadConfig(path, error)) << error;
        EXPECT_EQ(conv.GetResponsesLimit(), 10);
    }
}

TEST(ConverterJSONTest, ConfigVersionCheck) {
    std::string error;
    ConverterJSON conv;

    const std::string config_path = config_dir + "test_config.json";
    ASSERT_TRUE(conv.LoadConfig(config_path, error)) << error;

    EXPECT_TRUE(conv.CheckConfigVersion("1.0"));
    EXPECT_FALSE(conv.CheckConfigVersion("2.0"));
}

TEST(ConverterJSONTest, SaveAnswers_CreatesCorrectJSONFile) {
    ConverterJSON conv;

    std::vector<std::string> requests = {
        "milk water",
        "capital london",
        "americano"
    };

    std::vector<std::vector<RelativeIndex>> answers = {
        { {0, 1.0f}, {1, 0.5f} },
        { {2, 0.9f} },
        {}  // No results
    };

    const std::string answer_filename = "test_answers.json";

    ASSERT_TRUE(conv.SaveAnswers(answer_filename, requests, answers));

    std::ifstream file(answer_filename);
    ASSERT_TRUE(file.is_open());

    json result_json;
    file >> result_json;

    ASSERT_TRUE(result_json.contains("answers"));
    ASSERT_EQ(result_json["answers"].size(), 3);

    // First request
    auto a0 = result_json["answers"][0];
    EXPECT_EQ(a0["request"], "milk water");
    EXPECT_TRUE(a0["result"]);
    ASSERT_EQ(a0["relevance"].size(), 2);
    EXPECT_EQ(a0["relevance"][0]["doc_id"], 0);
    EXPECT_FLOAT_EQ(a0["relevance"][0]["rank"], 1.0f);
    EXPECT_EQ(a0["relevance"][1]["doc_id"], 1);
    EXPECT_FLOAT_EQ(a0["relevance"][1]["rank"], 0.5f);

    // Second request
    auto a1 = result_json["answers"][1];
    EXPECT_EQ(a1["request"], "capital london");
    EXPECT_TRUE(a1["result"]);
    ASSERT_EQ(a1["relevance"].size(), 1);
    EXPECT_EQ(a1["relevance"][0]["doc_id"], 2);
    EXPECT_FLOAT_EQ(a1["relevance"][0]["rank"], 0.9f);

    // Third request
    auto a2 = result_json["answers"][2];
    EXPECT_EQ(a2["request"], "americano");
    EXPECT_FALSE(a2["result"]);

    file.close();
    std::remove(answer_filename.c_str());
}
