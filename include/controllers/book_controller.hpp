// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#pragma once
#include "global.hpp"

#include "controllers/event_mgr.hpp"
#include "models/epub.hpp"
#include "models/page_locs.hpp"

/**
 * @brief Controller for the Book Reading view
 * 
 * Manages the interaction between the user and the ebook content.
 * Handles page navigation, gestures (swipe/tap), and bookmarking.
 */
class BookController
{
  public:
    BookController() :
      current_page_id(PageLocs::PageId(0, 0))
    { }

    /**
     * @brief Handle user input events
     * 
     * Processes touch gestures for page turning, menu access, and bookmarks.
     * 
     * @param event The input event
     */
    void input_event(const EventMgr::Event & event);

    /**
     * @brief Called when the view becomes active
     * 
     * Loads the current page location and renders the page.
     */
    void enter();

    /**
     * @brief Called when leaving the view
     * 
     * @param going_to_deep_sleep True if the device is about to sleep
     */
    void leave(bool going_to_deep_sleep = false);

    /**
     * @brief Open an EPUB book file
     * 
     * @param book_title Title of the book
     * @param book_filename Path to the .epub file
     * @param page_id Location to jump to (optional)
     * @return true if opened successfully
     * @return false if file error
     */
    bool open_book_file(const std::string & book_title, 
                        const std::string & book_filename, 
                        const PageLocs::PageId & page_id);

    void goto_page();

    inline const PageLocs::PageId & get_current_page_id() { return current_page_id; }
    inline void set_current_page_id(const PageLocs::PageId & page_id) { current_page_id = page_id; }

  private:
    static constexpr char const * TAG = "BookController";

    PageLocs::PageId current_page_id;
    bool             keypad_is_shown = false;
    std::string      entered_page_num_str;
};

#if __BOOK_CONTROLLER__
  BookController book_controller;
#else
  extern BookController book_controller;
#endif
