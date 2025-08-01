#include "gtest/gtest.h"
#include "InvertedIndex.h"
#include <vector>
#include <string>

using namespace std;

TEST(InvertedIndexTest, BasicIndexing) {
    InvertedIndex idx;
    vector<string> docs = {
        "milk sugar salt",
        "milk a milk b milk c milk d"
    };
    idx.updateDocumentBaseFromStrings(docs);

    vector<Entry> expected_milk = {{0, 1}, {1, 4}};
    vector<Entry> expected_sugar = {{0, 1}};
    vector<Entry> expected_a = {{1, 1}};
    vector<Entry> expected_missing;

    EXPECT_EQ(idx.getWordCount("milk"), expected_milk);
    EXPECT_EQ(idx.getWordCount("sugar"), expected_sugar);
    EXPECT_EQ(idx.getWordCount("a"), expected_a);
    EXPECT_EQ(idx.getWordCount("missing"), expected_missing);
}

TEST(InvertedIndexTest, EmptyDocs) {
    InvertedIndex idx;
    vector<string> docs;
    idx.updateDocumentBaseFromStrings(docs);

    EXPECT_TRUE(idx.getWordCount("anyword").empty());
}

TEST(InvertedIndexTest, MultipleDocsWordCounts) {
    InvertedIndex idx;
    vector<string> docs = {
        "apple orange apple banana",
        "banana apple apple orange orange",
        "grape"
    };
    idx.updateDocumentBaseFromStrings(docs);

    vector<Entry> expected_apple = {{0, 2}, {1, 2}};
    vector<Entry> expected_orange = {{0, 1}, {1, 2}};
    vector<Entry> expected_banana = {{0, 1}, {1, 1}};
    vector<Entry> expected_grape = {{2, 1}};

    EXPECT_EQ(idx.getWordCount("apple"), expected_apple);
    EXPECT_EQ(idx.getWordCount("orange"), expected_orange);
    EXPECT_EQ(idx.getWordCount("banana"), expected_banana);
    EXPECT_EQ(idx.getWordCount("grape"), expected_grape);
}

TEST(InvertedIndexTest, CaseSensitivity) {
    InvertedIndex idx;
    vector<string> docs = {
        "Apple apple APPLE"
    };
    idx.updateDocumentBaseFromStrings(docs);

    vector<Entry> expected_apple = {{0, 1}};
    vector<Entry> expected_Apple = {{0, 1}};
    vector<Entry> expected_APPLE = {{0, 1}};

    // Проверяем чувствительность к регистру — возвращаем отдельно для каждого варианта
    EXPECT_EQ(idx.getWordCount("apple"), expected_apple);
    EXPECT_EQ(idx.getWordCount("Apple"), expected_Apple);
    EXPECT_EQ(idx.getWordCount("APPLE"), expected_APPLE);
}
