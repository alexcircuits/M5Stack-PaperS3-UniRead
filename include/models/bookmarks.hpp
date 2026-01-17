// Copyright (c) 2024
//
// MIT License. Look at file licenses.txt for details.

#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <cstdio>
#include <cstring>
#include <cstdlib>

class Bookmarks
{
  public:
    struct Entry {
      std::string book_path;
      int         page_idx; // Changed from page_number to page_index to match internal representation
      std::string timestamp;
    };

    static Bookmarks & get() {
      static Bookmarks instance;
      return instance;
    }

    bool setup() {
       load();
       return true; 
    }

    void add(const std::string & book_path, int page_idx) {
      std::lock_guard<std::mutex> lock(mutex_);
      // Check if already exists
      for (const auto & entry : entries_) {
        if (entry.book_path == book_path && entry.page_idx == page_idx) return;
      }
      
      entries_.push_back({book_path, page_idx, ""});
      save();
    }

    void remove(const std::string & book_path, int page_idx) {
      std::lock_guard<std::mutex> lock(mutex_);
      for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        if (it->book_path == book_path && it->page_idx == page_idx) {
          entries_.erase(it);
          save();
          return;
        }
      }
    }

    bool has_bookmark(const std::string & book_path, int page_idx) const {
      for (const auto & entry : entries_) {
        if (entry.book_path == book_path && entry.page_idx == page_idx) return true;
      }
      return false;
    }

    std::vector<Entry> get_bookmarks_for_book(const std::string & book_path) const {
      std::vector<Entry> result;
      for (const auto & entry : entries_) {
        if (entry.book_path == book_path) {
          result.push_back(entry);
        }
      }
      return result;
    }
    
    void save() {
      FILE * f = fopen(BOOKMARKS_FILE, "w");
      if (!f) return;
      for (const auto & entry : entries_) {
        fprintf(f, "%s|%d\n", entry.book_path.c_str(), entry.page_idx);
      }
      fclose(f);
    }

    void load() {
      std::lock_guard<std::mutex> lock(mutex_);
      entries_.clear();
      
      FILE * f = fopen(BOOKMARKS_FILE, "r");
      if (!f) return;
      
      char line[512];
      while (fgets(line, sizeof(line), f)) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        
        // Parse: book_path|page_idx
        char * sep = strchr(line, '|');
        if (!sep) continue;
        
        *sep = '\0';
        std::string book_path = line;
        int page_idx = atoi(sep + 1);
        
        entries_.push_back({book_path, page_idx, ""});
      }
      fclose(f);
    }

  private:
    static constexpr const char * BOOKMARKS_FILE = "/sdcard/bookmarks.txt";
    Bookmarks() {}
    std::vector<Entry> entries_;
    mutable std::mutex mutex_;
};

extern Bookmarks & bookmarks;
