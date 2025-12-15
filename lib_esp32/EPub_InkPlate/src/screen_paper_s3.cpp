#define __SCREEN__ 1
#include "screen.hpp"

#if defined(BOARD_TYPE_PAPER_S3)

extern "C" {
  #include <epdiy.h>
  #include <epd_highlevel.h>
  #include <epd_display.h>
}

#include <FastEPD.h>

// Board definition implemented in PaperS3Support/EpdiyPaperS3Board.c
extern "C" {
  extern const EpdBoardDefinition paper_s3_board;
}

#ifndef EPD_WIDTH
#define EPD_WIDTH 960
#endif

#ifndef EPD_HEIGHT
#define EPD_HEIGHT 540
#endif

static FASTEPD s_epd;
static bool s_epd_initialized = false;
static uint8_t *s_framebuffer = nullptr;
static bool s_force_full = true;
static int16_t s_partial_count = 0;
static const int16_t PARTIAL_COUNT_ALLOWED = 10;
static int s_temperature = 20; // TODO: hook real temperature sensor

Screen Screen::singleton;

uint16_t Screen::width  = EPD_WIDTH;
uint16_t Screen::height = EPD_HEIGHT;

void Screen::clear()
{
  if (!s_epd_initialized) return;
  s_epd.fillScreen(0x0F);
}

void Screen::update(bool no_full)
{
  if (!s_epd_initialized) return;

  if (s_force_full) {
    s_epd.fullUpdate(CLEAR_FAST, true, nullptr);
    s_force_full = false;
    s_partial_count = PARTIAL_COUNT_ALLOWED;
    return;
  }

  if (no_full) {
    s_epd.fullUpdate(CLEAR_NONE, true, nullptr);
    s_partial_count = 0;
    return;
  }

  if (s_partial_count <= 0) {
    s_epd.fullUpdate(CLEAR_FAST, true, nullptr);
    s_partial_count = PARTIAL_COUNT_ALLOWED;
  } else {
    s_epd.fullUpdate(CLEAR_NONE, true, nullptr);
    s_partial_count--;
  }
}

void Screen::force_full_update()
{
  s_force_full = true;
  s_partial_count = 0;
}

void Screen::setup(PixelResolution resolution, Orientation orientation)
{
  if (!s_epd_initialized) {
    int rc = s_epd.initPanel(BB_PANEL_M5PAPERS3, 20000000);
    if (rc != BBEP_SUCCESS) {
      return;
    }

    s_epd.setMode(BB_MODE_4BPP);
    s_epd.setRotation(90);
    s_framebuffer = s_epd.currentBuffer();

    s_epd.fillScreen(0x0F);
    s_epd.fullUpdate(CLEAR_FAST, true, nullptr);
    s_epd_initialized = true;
    s_force_full = false;
    s_partial_count = PARTIAL_COUNT_ALLOWED;
  }

  // On Paper S3 we always drive the panel in grayscale (4-bit via epdiy).
  // Ignore the stored pixel resolution setting and force the non-ONE_BIT
  // path so glyphs/bitmaps are generated as grayscale only.
  (void)resolution;
  set_pixel_resolution(PixelResolution::THREE_BITS, true);
  set_orientation(orientation);
  clear();
}

void Screen::set_pixel_resolution(PixelResolution resolution, bool force)
{
  if (force || (pixel_resolution != resolution)) {
    pixel_resolution = resolution;
  }
}

void Screen::set_orientation(Orientation orient)
{
  orientation = orient;
  // With EPD_ROT_INVERTED_PORTRAIT set at init time, epdiy exposes a
  // logical portrait space of 540x960 (EPD_HEIGHT x EPD_WIDTH). Keep the
  // logical Screen dimensions fixed to that space regardless of the
  // orientation enum so the layout engine can use the full page.
  width  = EPD_HEIGHT;  // 540
  height = EPD_WIDTH;   // 960
}

static inline uint8_t map_gray(uint8_t v)
{
  // Convert an 8-bit grayscale value (0=black..255=white) into an epdiy
  // API color byte (upper nibble significant).
  return (uint8_t)(v & 0xF0);
}

static inline uint8_t gray8_to_nibble(uint8_t v)
{
  // 0 (black) .. 255 (white) => 0..15
  return (uint8_t)(v >> 4);
}

static inline uint8_t alpha8_to_nibble(uint8_t a)
{
  // Alpha 0 (transparent) .. 255 (opaque) => 15..0 (white..black)
  return (uint8_t)(15 - (a >> 4));
}

static inline uint8_t gray3_to_nibble(uint8_t v)
{
  // 3-bit grayscale 0..7 => 0..15
  return (uint8_t)((v * 15 + 3) / 7);
}

static inline void set_pixel_nibble_physical(uint16_t x, uint16_t y, uint8_t nibble)
{
  // Write a 4-bpp pixel directly into the epdiy framebuffer.
  // x: 0..EPD_WIDTH-1 (960), y: 0..EPD_HEIGHT-1 (540)
  uint8_t * buf_ptr = &s_framebuffer[y * (EPD_WIDTH / 2) + (x >> 1)];
  if (x & 1) {
    *buf_ptr = (uint8_t)((*buf_ptr & 0x0F) | ((nibble & 0x0F) << 4));
  } else {
    *buf_ptr = (uint8_t)((*buf_ptr & 0xF0) | (nibble & 0x0F));
  }
}

static inline void set_pixel_nibble_screen(uint16_t x, uint16_t y, uint8_t nibble)
{
  // Screen coordinates for Paper S3 are logical portrait (width=540, height=960)
  // with epdiy set to EPD_ROT_INVERTED_PORTRAIT.
  // The equivalent physical coordinates in the 960x540 framebuffer are:
  //   x_phys = y
  //   y_phys = (EPD_HEIGHT - 1) - x
  set_pixel_nibble_physical(y, (uint16_t)((EPD_HEIGHT - 1) - x), nibble);
}

void Screen::draw_bitmap(const unsigned char * bitmap_data, Dim dim, Pos pos)
{
  if (!s_epd_initialized || (bitmap_data == nullptr)) return;

  const int32_t x0 = pos.x;
  const int32_t y0 = pos.y;
  const int32_t x1 = x0 + dim.width;
  const int32_t y1 = y0 + dim.height;

  const int32_t x_start = (x0 < 0) ? 0 : x0;
  const int32_t y_start = (y0 < 0) ? 0 : y0;
  const int32_t x_end = (x1 > (int32_t)width) ? (int32_t)width : x1;
  const int32_t y_end = (y1 > (int32_t)height) ? (int32_t)height : y1;

  if (x_start >= x_end || y_start >= y_end) return;

  for (int32_t x = x_start; x < x_end; ++x) {
    const int32_t sx = x - x0;
    for (int32_t y = y_start; y < y_end; ++y) {
      const int32_t sy = y - y0;
      const uint32_t p = (uint32_t)sy * (uint32_t)dim.width + (uint32_t)sx;
      const uint8_t v = bitmap_data[p];
      set_pixel_nibble_screen((uint16_t)x, (uint16_t)y, gray8_to_nibble(v));
    }
  }
}

void Screen::draw_glyph(const unsigned char * bitmap_data, Dim dim, Pos pos, uint16_t pitch)
{
  if (!s_epd_initialized || (bitmap_data == nullptr)) return;

  const int32_t x0 = pos.x;
  const int32_t y0 = pos.y;
  const int32_t x1 = x0 + dim.width;
  const int32_t y1 = y0 + dim.height;

  const int32_t x_start = (x0 < 0) ? 0 : x0;
  const int32_t y_start = (y0 < 0) ? 0 : y0;
  const int32_t x_end = (x1 > (int32_t)width) ? (int32_t)width : x1;
  const int32_t y_end = (y1 > (int32_t)height) ? (int32_t)height : y1;

  if (x_start >= x_end || y_start >= y_end) return;

  for (int32_t x = x_start; x < x_end; ++x) {
    const int32_t sx = x - x0;
    for (int32_t y = y_start; y < y_end; ++y) {
      const int32_t sy = y - y0;
      const uint8_t a = bitmap_data[(uint32_t)sy * (uint32_t)pitch + (uint32_t)sx];
      if (!a) continue;
      const uint8_t nib = alpha8_to_nibble(a);
      if (nib == 0x0F) continue;
      set_pixel_nibble_screen((uint16_t)x, (uint16_t)y, nib);
    }
  }
}

void Screen::draw_rectangle(Dim dim, Pos pos, uint8_t color)
{
  if (!s_epd_initialized) return;

  const int32_t x0 = pos.x;
  const int32_t y0 = pos.y;
  const int32_t x1 = x0 + dim.width;
  const int32_t y1 = y0 + dim.height;
  const int32_t x_start = (x0 < 0) ? 0 : x0;
  const int32_t y_start = (y0 < 0) ? 0 : y0;
  const int32_t x_end = (x1 > (int32_t)width) ? (int32_t)width : x1;
  const int32_t y_end = (y1 > (int32_t)height) ? (int32_t)height : y1;
  if (x_start >= x_end || y_start >= y_end) return;

  const uint8_t nib = gray3_to_nibble(color);

  // Top and bottom edges
  const uint16_t top_y = (uint16_t)y_start;
  const uint16_t bottom_y = (uint16_t)(y_end - 1);
  for (int32_t x = x_start; x < x_end; ++x) {
    set_pixel_nibble_screen((uint16_t)x, top_y, nib);
    set_pixel_nibble_screen((uint16_t)x, bottom_y, nib);
  }
  // Left and right edges
  const uint16_t left_x = (uint16_t)x_start;
  const uint16_t right_x = (uint16_t)(x_end - 1);
  for (int32_t y = y_start; y < y_end; ++y) {
    set_pixel_nibble_screen(left_x, (uint16_t)y, nib);
    set_pixel_nibble_screen(right_x, (uint16_t)y, nib);
  }
}

void Screen::draw_round_rectangle(Dim dim, Pos pos, uint8_t color)
{
  // Approximate with a simple rectangle for now.
  draw_rectangle(dim, pos, color);
}

void Screen::colorize_region(Dim dim, Pos pos, uint8_t color)
{
  if (!s_epd_initialized) return;

  const int32_t x0 = pos.x;
  const int32_t y0 = pos.y;
  const int32_t x1 = x0 + dim.width;
  const int32_t y1 = y0 + dim.height;
  const int32_t x_start = (x0 < 0) ? 0 : x0;
  const int32_t y_start = (y0 < 0) ? 0 : y0;
  const int32_t x_end = (x1 > (int32_t)width) ? (int32_t)width : x1;
  const int32_t y_end = (y1 > (int32_t)height) ? (int32_t)height : y1;
  if (x_start >= x_end || y_start >= y_end) return;

  const uint8_t nib = gray3_to_nibble(color);

  for (int32_t x = x_start; x < x_end; ++x) {
    for (int32_t y = y_start; y < y_end; ++y) {
      set_pixel_nibble_screen((uint16_t)x, (uint16_t)y, nib);
    }
  }
}

#endif // BOARD_TYPE_PAPER_S3
