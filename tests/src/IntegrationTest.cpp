#include "gtest/gtest.h"
#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include "SearchServer.h"
#include "json.hpp"

#include <fstream>
#include <string>

using json = nlohmann::json;

TEST(IntegrationTest, FullCycle) {
    ConverterJSON converter;
    std::string error;

    ASSERT_TRUE(converter.LoadConfig("../tests/config/test_config.json", error)) << error;
    ASSERT_TRUE(converter.LoadRequests("../tests/config/test_requests.json", error)) << error;

    auto documents = converter.GetTextDocuments();
    ASSERT_FALSE(documents.empty()) << "Documents list should not be empty";

    auto requests = converter.GetRequests();
    ASSERT_FALSE(requests.empty()) << "Requests list should not be empty";

    InvertedIndex index;
    index.updateDocumentBase(documents);

    SearchServer server(index);
    auto results = server.search(requests);

    ASSERT_TRUE(converter.putAnswers(requests, results));

    std::ifstream answers_file("answers.json");
    ASSERT_TRUE(answers_file.is_open()) << "Cannot open answers.json";

    json answers_json;
    answers_file >> answers_json;

    ASSERT_TRUE(answers_json.contains("answers"));
    auto& answers_array = answers_json["answers"];
    ASSERT_EQ(answers_array.size(), requests.size());

    auto& first_answer = answers_array[0];
    ASSERT_TRUE(first_answer.contains("request"));
    ASSERT_TRUE(first_answer.contains("result"));

    if (first_answer["result"].get<bool>()) {
        ASSERT_TRUE(first_answer.contains("relevance"));
        for (auto& rel : first_answer["relevance"]) {
            ASSERT_TRUE(rel.contains("doc_id"));
            ASSERT_TRUE(rel.contains("rank"));
            ASSERT_TRUE(rel["doc_id"].is_number_integer());
            ASSERT_TRUE(rel["rank"].is_number());
        }
    }
}
