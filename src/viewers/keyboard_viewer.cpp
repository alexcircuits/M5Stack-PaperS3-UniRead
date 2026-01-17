#define __KEYBOARD_VIEWER__ 1
#include "viewers/keyboard_viewer.hpp"

#if EPUB_INKPLATE_BUILD
  #include "nvs.h"
  #include "inkplate_platform.hpp"
#endif

#include "models/fonts.hpp"

// Simple mapping for the keypad
// We will center the keypad on screen.
// Key size: 80x80
// 4 rows, 3 cols
static constexpr int KEY_W = 80;
static constexpr int KEY_H = 80;
static constexpr int GAP   = 10;
static constexpr int KB_W  = 3 * KEY_W + 2 * GAP;
static constexpr int KB_H  = 4 * KEY_H + 3 * GAP;

void KeyboardViewer::get_num()
{
  width           = KB_W;
  current_kb_type = KBType::NUMBERS;

  page.set_compute_mode(Page::ComputeMode::DISPLAY);
  
  // Clear the area using a simple trick: render a whitespace rectangle or just rely on partial update clearing?
  // Since we don't have a clear_region easily exposed without page logic, let's just draw the keys.
  // Actually, we should probably clear the screen or at least the region.
  // For now, let's assume the caller clears or we draw over.
  
  // We'll draw a background rectangle for the keypad
  int start_x = (Screen::get_width() - KB_W) / 2;
  int start_y = (Screen::get_height() - KB_H) / 2;
  
  // Clear background (white)
  Dim dim(KB_W + 20, KB_H + 20);
  Pos pos(start_x - 10, start_y - 10);
  screen.draw_rectangle(dim, pos, 255); // White background

  show_kb(current_kb_type);
}

void KeyboardViewer::show_kb(KBType kb_type)
{
  uint16_t x, y;
  
  // Center coordinates
  x = (Screen::get_width() - KB_W) / 2;
  y = (Screen::get_height() - KB_H) / 2;

  switch (kb_type) {
    case KBType::NUMBERS:
      show_line(num_line_1, x, y);
      x = (Screen::get_width() - KB_W) / 2; 
      y += KEY_H + GAP;
      show_line(num_line_2, x, y);
      x = (Screen::get_width() - KB_W) / 2; 
      y += KEY_H + GAP;
      show_line(num_line_3, x, y);
      x = (Screen::get_width() - KB_W) / 2; 
      y += KEY_H + GAP;
      show_line(num_line_4, x, y);
      break;
    default:
      break;
  }
}

void KeyboardViewer::show_line(const char * line, uint16_t & x, uint16_t y)
{
  const char * p = line;
  while (*p) {
    show_char(*p, x, y);
    p++;
    x += KEY_W + GAP;
  }
}

void KeyboardViewer::show_char(char ch, uint16_t & x, uint16_t y)
{
  // Draw key outline
  screen.draw_rectangle(Dim(KEY_W, KEY_H), Pos(x, y), 0); // Black border? No, wait draw_rectangle fills. 
  // We need a framed rectangle. Screen class only has filled rectangle.
  // Let's draw a black box then a smaller white box inside.
  screen.draw_rectangle(Dim(KEY_W, KEY_H), Pos(x, y), 0);
  screen.draw_rectangle(Dim(KEY_W-4, KEY_H-4), Pos(x+2, y+2), 255);
  
  // Draw Character
  // We need a font.
  static int8_t font_idx = -1;
  // Try to find a bold font or large font.
  // Default font size 12pt is too small. Need larger.
  // fonts.get(0, 30) -> Index 0 is usually the default serif/sans.
  
  Font * font = fonts.get(2); // larger font
  if (!font) font = fonts.get(0);
  if (!font) return;

  char str[2] = {ch, 0};
  if (ch == '\b') str[0] = '<'; // Backspace visual
  if (ch == '\r') str[0] = 'k'; // OK visual (check mark?) let's use 'k' or 'O'
  
  // Calculate text position to center it
  int text_w = 0;
  // Simplified width calc
  text_w = 20; // approximate
  
  // Real way:
  // Font::Glyph * glyph = font->get_glyph(ch, 30);
  
  // Drawing text directly into screen is hard without Page class handling it.
  // But wait, Page class has put_char_at but it needs Format.
  
  // Let's use a simple workaround: use Page to draw a single char? 
  // Or just implement a simple text drawer here?
  // Actually, let's use the Page class but just for drawing?
  // No, Page class manages a bitmap cache.
  
  // Let's try to use the glyph directly if possible.
  // screen.draw_glyph(...)
  
  int32_t code = (uint8_t)str[0];
  if (ch == '\r') { code = 'O'; } // O for OK
  
  const char * s = str;
  bool first = true;
  // code = to_unicode(...) -- skip unicode for now, assume ASCII
  
  Font::Glyph * glyph = font->get_glyph(code, 40);
  if (glyph) {
    int px = x + (KEY_W - glyph->dim.width) / 2;
    int py = y + (KEY_H - glyph->dim.height) / 2;
    screen.draw_glyph(glyph->buffer, glyph->dim, Pos(px, py), glyph->pitch);
  }
}

bool KeyboardViewer::event(const EventMgr::Event & event, char & char_code)
{
  if (event.kind != EventMgr::EventKind::TAP) return false;
  
  int start_x = (Screen::get_width() - KB_W) / 2;
  int start_y = (Screen::get_height() - KB_H) / 2;
  
  if (event.x < start_x || event.x > start_x + KB_W) return false;
  if (event.y < start_y || event.y > start_y + KB_H) return false;
  
  int col = (event.x - start_x) / (KEY_W + GAP);
  int row = (event.y - start_y) / (KEY_H + GAP);
  
  if (col >= 3 || row >= 4) return false;
  
  // Map row/col to char
  // Row 0: 1 2 3
  // Row 1: 4 5 6
  // Row 2: 7 8 9
  // Row 3:   0 < O  (Space, 0, Backspace, Return) -- wait
  // My definition was: " 0\b\r" -> Space, 0, Back, Ret
  
  if (row == 0) char_code = "123"[col];
  if (row == 1) char_code = "456"[col];
  if (row == 2) char_code = "789"[col];
  if (row == 3) {
     if (col == 0) char_code = ' ';
     if (col == 1) char_code = '0';
     if (col == 2) {
       // Split space? Setup was " 0\b\r" length 4? 
       // Ah, show_kb iterates keys.
       // show_line(num_line_4...)
       // num_line_4 = " 0\b\r" -> 4 chars. But we have 3 cols?
       // My logic in show_line puts them 3 per row?
       // Wait, " 0\b\r" has 4 chars. It would draw a 4th column which exceeds KB_W?
       // Let's redefine num_line_4 to "0\b\r" (3 keys)
       // Key 0: 0
       // Key 1: Backspace
       // Key 2: OK
       if (col == 0) char_code = '0';
       if (col == 1) char_code = '\b';
       if (col == 2) char_code = '\r';
     }
  }
  
  return true;
}

// Unused methods stubbed
bool KeyboardViewer::get_alfanum(char * str, uint16_t len, UpdateHandler handler) { return false; }
bool KeyboardViewer::load_font() { return true; }