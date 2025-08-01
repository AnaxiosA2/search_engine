#include "gtest/gtest.h"
#include "configUtils.h"
#include <string>

using namespace std;

TEST(UtilsTest, NormalizeDash) {
    wstring input = L"file–name.txt"; // содержит длинное тире (U+2013)
    wstring expected = L"file-name.txt";
    EXPECT_EQ(normalize_dash(input), expected);
}

TEST(UtilsTest, WstringToUtf8AndBack) {
    wstring original = L"тестовая строка 123";
    string utf8 = wstring_to_utf8(original);
    wstring back = utf8_to_wstring(utf8);

    EXPECT_EQ(original, back);
}
