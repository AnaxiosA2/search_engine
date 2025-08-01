#include "gtest/gtest.h"
#include "SearchServer.h"
#include "InvertedIndex.h"
#include <vector>
#include <string>

using namespace std;

TEST(SearchServerTest, BasicSearch) {
    vector<string> docs = {
        "milk sugar salt",
        "milk a milk b milk c milk d"
    };
    InvertedIndex idx;
    idx.updateDocumentBaseFromStrings(docs);
    SearchServer server(idx);

    vector<string> queries = {"milk", "salt", "missing"};
    auto results = server.search(queries);

    // milk appears in docs 0 and 1
    EXPECT_FALSE(results[0].empty());
    EXPECT_EQ(results[0][0].doc_id, 1); // doc 1 has milk 4 times, more relevant
    EXPECT_EQ(results[0][1].doc_id, 0);

    // salt appears only in doc 0
    EXPECT_EQ(results[1].size(), 1);
    EXPECT_EQ(results[1][0].doc_id, 0);

    // missing word returns empty vector
    EXPECT_TRUE(results[2].empty());
}

TEST(SearchServerTest, MaxResponsesLimit) {
    vector<string> docs = {
        "apple banana cherry",
        "banana apple cherry",
        "cherry apple banana",
        "apple cherry banana"
    };
    InvertedIndex idx;
    idx.updateDocumentBaseFromStrings(docs);
    SearchServer server(idx);

    server.setMaxResponses(2); // Ограничиваем максимум результатов двумя

    vector<string> queries = {"apple"};
    auto results = server.search(queries);

    // Проверяем, что количество результатов не больше 2
    EXPECT_LE(results[0].size(), 2);
}
