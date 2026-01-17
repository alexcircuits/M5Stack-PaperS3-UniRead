#if TESTING

#include "gtest/gtest.h"
#include "models/bookmarks.hpp"

// Test fixture
class BookmarksTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

TEST_F(BookmarksTest, AddAndCheck) {
    Bookmarks & b = Bookmarks::get();
    std::string book = "/books/test.epub";
    int page = 10;

    // Ensure clean state
    if (b.has_bookmark(book, page)) {
        b.remove(book, page);
    }
    
    EXPECT_FALSE(b.has_bookmark(book, page));
    
    b.add(book, page);
    EXPECT_TRUE(b.has_bookmark(book, page));
}

TEST_F(BookmarksTest, Remove) {
    Bookmarks & b = Bookmarks::get();
    std::string book = "/books/test.epub";
    int page = 20;
    
    b.add(book, page);
    EXPECT_TRUE(b.has_bookmark(book, page));
    
    b.remove(book, page);
    EXPECT_FALSE(b.has_bookmark(book, page));
}

TEST_F(BookmarksTest, MultipleBooks) {
    Bookmarks & b = Bookmarks::get();
    std::string book1 = "/books/book1.epub";
    std::string book2 = "/books/book2.epub";
    int page = 5;
    
    b.add(book1, page);
    EXPECT_TRUE(b.has_bookmark(book1, page));
    EXPECT_FALSE(b.has_bookmark(book2, page));
    
    b.add(book2, page);
    EXPECT_TRUE(b.has_bookmark(book2, page));
}

#endif
