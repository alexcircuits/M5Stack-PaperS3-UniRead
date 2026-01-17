// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#define __BOOK_CONTROLLER__ 1
#include "controllers/book_controller.hpp"

#include "controllers/app_controller.hpp"
#include "controllers/books_dir_controller.hpp"
#include "models/epub.hpp"
#include "models/bookmarks.hpp"
#include "viewers/book_viewer.hpp"
#include "viewers/page.hpp"
#include "viewers/msg_viewer.hpp"

#if EPUB_INKPLATE_BUILD
  #include "nvs.h"
#endif

#include "viewers/keyboard_viewer.hpp"

void 
BookController::goto_page()
{
  keypad_is_shown = true;
  entered_page_num_str = "";
  keyboard_viewer.get_num();
}

void 
BookController::enter()
{ 
  LOG_D("===> Enter()...");

  page_locs.check_for_format_changes(epub.get_item_count(), current_page_id.itemref_index);
  const PageLocs::PageId * id = page_locs.get_page_id(current_page_id);
  if (id != nullptr) {
    current_page_id.itemref_index = id->itemref_index;
    current_page_id.offset        = id->offset;
  }
  else {
    current_page_id.itemref_index = 0;
    current_page_id.offset        = 0;
  }
  book_viewer.show_page(current_page_id);
}

void 
BookController::leave(bool going_to_deep_sleep)
{
  LOG_D("===> leave()...");
  
  books_dir_controller.save_last_book(current_page_id, going_to_deep_sleep);
}

bool
BookController::open_book_file(
  const std::string & book_title, 
  const std::string & book_filename, 
  const PageLocs::PageId & page_id)
{
  LOG_D("===> open_book_file()...");

  msg_viewer.show(MsgViewer::MsgType::BOOK, false, false, "Loading a book",
     "The book \" %s \" is loading. Please wait.", book_title.c_str());

  bool new_document = book_filename != epub.get_current_filename();

  if (new_document) page_locs.stop_document();

  if (epub.open_file(book_filename)) {
    if (new_document) {
      page_locs.start_new_document(epub.get_item_count(), page_id.itemref_index);
    }
    else {
      page_locs.check_for_format_changes(epub.get_item_count(), page_id.itemref_index);
    }
    book_viewer.init();
    const PageLocs::PageId * id = page_locs.get_page_id(page_id);
    if (id != nullptr) {
      current_page_id.itemref_index = id->itemref_index;
      current_page_id.offset        = id->offset;
      // book_viewer.show_page(current_page_id);
      return true;
    }
  }
  return false;
}

#if INKPLATE_6PLUS || TOUCH_TRIAL
  void 
  BookController::input_event(const EventMgr::Event & event)
  {
    if (keypad_is_shown) {
      char ch = 0;
      if (keyboard_viewer.event(event, ch)) {
        if (ch == '\r') {
          // OK/Enter
          if (!entered_page_num_str.empty()) {
             int page_num = atoi(entered_page_num_str.c_str());

             const PageLocs::PageId * pid = page_locs.get_page_id_from_nbr(page_num);
             if (pid != nullptr) {
               current_page_id = *pid;
             }
             else {
               // Should show "Invalid Page" message?
               // For now just stay on current page.
             }

             keypad_is_shown = false;
             book_viewer.show_page(current_page_id); // Refresh
          } else {
             keypad_is_shown = false;
             book_viewer.show_page(current_page_id);
          }
        } 
        else if (ch == '\b') {
          if (!entered_page_num_str.empty()) {
            entered_page_num_str.pop_back();
            keyboard_viewer.get_num(); // Redraw
            // We should draw the number too...
          }
        }
        else {
          entered_page_num_str += ch;
          // We need to show what is typed.
          // Since get_num just draws the keypad, we need to draw the text manually or add it to get_num.
          // For now, let's just log it or rely on blind typing? Blind typing is bad.
          // Let's assume keyboard_viewer draws it? No I didn't impl that.
        }
      }
      // If tap outside, cancel?
      else if (event.kind == EventMgr::EventKind::TAP) {
        // Cancel
        keypad_is_shown = false;
        book_viewer.show_page(current_page_id);
      }
      return;
    }

    const PageLocs::PageId * page_id;
    switch (event.kind) {
      case EventMgr::EventKind::SWIPE_RIGHT:
        if (event.y < (Screen::get_height() - 40)) {
          page_id = page_locs.get_prev_page_id(current_page_id);
          if (page_id != nullptr) {
            current_page_id.itemref_index = page_id->itemref_index;
            current_page_id.offset        = page_id->offset;
            book_viewer.show_page(current_page_id);
          }
        }
        else {
          page_id = page_locs.get_prev_page_id(current_page_id, 10);
          if (page_id != nullptr) {
            current_page_id.itemref_index = page_id->itemref_index;
            current_page_id.offset        = page_id->offset;
            book_viewer.show_page(current_page_id);
          }
        }
        break;
      
      case EventMgr::EventKind::SWIPE_DOWN:
        // Return to Library listing
        app_controller.set_controller(AppController::Ctrl::DIR);
        break;

      case EventMgr::EventKind::SWIPE_LEFT:
        if (event.y < (Screen::get_height() - 40)) {
          page_id = page_locs.get_next_page_id(current_page_id);
          if (page_id != nullptr) {
            current_page_id.itemref_index = page_id->itemref_index;
            current_page_id.offset        = page_id->offset;
            book_viewer.show_page(current_page_id);
          }
        }
        else {
          page_id = page_locs.get_next_page_id(current_page_id, 10);
          if (page_id != nullptr) {
            current_page_id.itemref_index = page_id->itemref_index;
            current_page_id.offset        = page_id->offset;
            book_viewer.show_page(current_page_id);
          }           
        }
        break;
      
      case EventMgr::EventKind::HOLD:
        {
          std::string filename = epub.get_current_filename();
          int page_idx = current_page_id.itemref_index; 
          
          if (bookmarks.has_bookmark(filename, page_idx)) {
             bookmarks.remove(filename, page_idx);
             msg_viewer.show(MsgViewer::MsgType::INFO, true, false, "Bookmark", "Bookmark Removed");
          } else {
             bookmarks.add(filename, page_idx);
             msg_viewer.show(MsgViewer::MsgType::INFO, true, false, "Bookmark", "Bookmark Added");
          }
        }
        break;

      case EventMgr::EventKind::TAP:
        if (event.y < (Screen::get_height() - 40)) {
          if (event.x < (Screen::get_width() / 3)) {
            page_id = page_locs.get_prev_page_id(current_page_id);
            if (page_id != nullptr) {
              current_page_id.itemref_index = page_id->itemref_index;
              current_page_id.offset        = page_id->offset;
              book_viewer.show_page(current_page_id);
            }
          }
          else if (event.x > ((Screen::get_width() / 3) * 2)) {
            page_id = page_locs.get_next_page_id(current_page_id);
            if (page_id != nullptr) {
              current_page_id.itemref_index = page_id->itemref_index;
              current_page_id.offset        = page_id->offset;
              book_viewer.show_page(current_page_id);
            }
          } else {           
            app_controller.set_controller(AppController::Ctrl::PARAM);
          }
        } else {           
          // Bottom area tap
          if ((event.x > (Screen::get_width() / 3)) && 
              (event.x < ((Screen::get_width() / 3) * 2))) {
            // Center tap: Go To Page
            goto_page();
          }
          else {
            // Left or Right corner tap: Open Menu
             app_controller.set_controller(AppController::Ctrl::PARAM);
          }
        }
        break;
        
      default:
        break;
    }
  }
#else
  void 
  BookController::input_event(const EventMgr::Event & event)
  {
    const PageLocs::PageId * page_id;
    switch (event.kind) {
      #if EXTENDED_CASE
        case EventMgr::EventKind::DBL_PREV:
      #else
        case EventMgr::EventKind::PREV:
      #endif
        page_id = page_locs.get_prev_page_id(current_page_id);
        if (page_id != nullptr) {
          current_page_id.itemref_index = page_id->itemref_index;
          current_page_id.offset        = page_id->offset;
          book_viewer.show_page(current_page_id);
        }
        break;

      #if EXTENDED_CASE
        case EventMgr::EventKind::PREV:
      #else
        case EventMgr::EventKind::DBL_PREV:
      #endif
        page_id = page_locs.get_prev_page_id(current_page_id, 10);
        if (page_id != nullptr) {
          current_page_id.itemref_index = page_id->itemref_index;
          current_page_id.offset        = page_id->offset;
          book_viewer.show_page(current_page_id);
        }
        break;

      #if EXTENDED_CASE
        case EventMgr::EventKind::DBL_NEXT:
      #else
        case EventMgr::EventKind::NEXT:
      #endif
        page_id = page_locs.get_next_page_id(current_page_id);
        if (page_id != nullptr) {
          current_page_id.itemref_index = page_id->itemref_index;
          current_page_id.offset        = page_id->offset;
          book_viewer.show_page(current_page_id);
        }
        break;

      #if EXTENDED_CASE
        case EventMgr::EventKind::NEXT:
      #else
        case EventMgr::EventKind::DBL_NEXT:
      #endif
        page_id = page_locs.get_next_page_id(current_page_id, 10);
        if (page_id != nullptr) {
          current_page_id.itemref_index = page_id->itemref_index;
          current_page_id.offset        = page_id->offset;
          book_viewer.show_page(current_page_id);
        }
        break;
      
      case EventMgr::EventKind::SELECT:
      case EventMgr::EventKind::DBL_SELECT:
        app_controller.set_controller(AppController::Ctrl::PARAM);
        break;
        
      case EventMgr::EventKind::NONE:
        break;
    }
  }
#endif
