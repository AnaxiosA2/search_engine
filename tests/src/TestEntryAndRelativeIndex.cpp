#include "gtest/gtest.h"
#include "Entry.h"
#include "RelativeIndex.h"

TEST(EntryTest, EqualityOperator) {
    Entry e1{1, 10};
    Entry e2{1, 10};
    Entry e3{2, 10};
    Entry e4{1, 5};

    EXPECT_TRUE(e1 == e2);
    EXPECT_FALSE(e1 == e3);
    EXPECT_FALSE(e1 == e4);
}

TEST(RelativeIndexTest, EqualityOperator) {
    RelativeIndex r1{0, 0.5f};
    RelativeIndex r2{0, 0.5f};
    RelativeIndex r3{1, 0.5f};
    RelativeIndex r4{0, 0.7f};

    EXPECT_TRUE(r1 == r2);
    EXPECT_FALSE(r1 == r3);
    EXPECT_FALSE(r1 == r4);
}
