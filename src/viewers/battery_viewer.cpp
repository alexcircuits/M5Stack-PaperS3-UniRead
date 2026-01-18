// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#define __BATTERY_VIEWER__ 1
#include "viewers/battery_viewer.hpp"

// Paper S3 Battery Implementation
#if defined(BOARD_TYPE_PAPER_S3)
  #include "viewers/page.hpp"
  #include "models/config.hpp"
  #include "screen.hpp"
  #include "logging.hpp"

  #include "esp_adc/adc_oneshot.h"
  #include "esp_adc/adc_cali.h"
  #include "esp_adc/adc_cali_scheme.h"

  #include <cstring>
  #include <stdio.h>

  static constexpr char const * TAG = "BatteryViewer";

  // M5Stack Paper S3: Battery voltage is on GPIO 36 (ADC1 CH 0)
  // Voltage divider is 1/2.
  static adc_oneshot_unit_handle_t adc1_handle = NULL;
  static bool adc_initialized = false;
  static float last_voltage = 0.0f;

  static void init_adc() {
    if (adc_initialized) return;

    adc_oneshot_unit_init_cfg_t init_config1 = {
      .unit_id = ADC_UNIT_1,
    };
    esp_err_t err = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (err == ESP_OK) {
        adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);
        adc_initialized = true;
    } else {
        LOG_E("ADC Init failed");
    }
  }

  static float read_battery_voltage() {
    if (!adc_initialized) init_adc();
    if (!adc_initialized) return 3.7f; // Dummy value

    int adc_raw;
    if (adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_raw) == ESP_OK) {
        // M5 Paper S3 schematic: Divider is 100k / 100k -> factor 2.
        // Raw ADC is approx voltage. 
        // Using rough estimation: 3.3V reference? 
        // Actually, let's assume standard calibration isn't available and use direct ratio for now or simple formula.
        // V_bat = (adc_raw / 4095.0) * 3.3 * 2.0; (approx)
        
        // Calibration would be better, but let's start simple.
        float v = (adc_raw * 3.3f * 2.0f) / 4095.0f;
        // Simple smoothing
        if (last_voltage == 0.0f) last_voltage = v;
        else last_voltage = (last_voltage * 0.8f) + (v * 0.2f);
        
        return last_voltage;
    }
    return 0.0f;
  }

  void
  BatteryViewer::show()
  {
    int8_t view_mode;
    config.get(Config::Ident::BATTERY, &view_mode);

    if (view_mode == 0) return;

    float voltage = read_battery_voltage();

    LOG_D("Battery voltage: %5.3f", voltage);

    Page::Format fmt = {
      .line_height_factor = 1.0,
      .font_index         = 1,
      .font_size          = 9,
      .indent             = 0,
      .margin_left        = 0,
      .margin_right       = 0,
      .margin_top         = 0,
      .margin_bottom      = 0,
      .screen_left        = 10,
      .screen_right       = 10,
      .screen_top         = 10,
      .screen_bottom      = 10,
      .width              = 0,
      .height             = 0,
      .vertical_align     = 0,
      .trim               = true,
      .pre                = false,
      .font_style         = Fonts::FaceStyle::NORMAL,
      .align              = CSS::Align::LEFT,
      .text_transform     = CSS::TextTransform::NONE,
      .display            = CSS::Display::INLINE
    };

    // Show battery icon
    Font * font = fonts.get(0);
    if (font == nullptr) {
      LOG_E("Internal error (Drawings Font not available!");
      return;
    }

    // Clear background to prevent ghosting
    // Assuming height is approx 20px for status bar area
    // page.clear_region(Dim(100, 25), Pos(0, Screen::get_height() - 25));
    // Actually, let's use a safe calculated height based on font
    int16_t h = font->get_chars_height(9) + 10;
    page.clear_region(Dim(100, h), Pos(0, Screen::get_height() - h));

    // Map 3.6V .. 4.2V to 0..4 icons
    float   value = ((voltage - 3.6f) * 4.0f) / (4.2f - 3.6f);
    if (value < 0) value = 0;
    int16_t icon_index =  (int16_t)value; 
    if (icon_index > 4) icon_index = 4;

    static constexpr char icons[5] = { '0', '1', '2', '3', '4' };
    Font::Glyph * glyph = font->get_glyph(icons[icon_index], 9);
    
    Pos pos;
    pos.y = Screen::get_height() + font->get_descender_height(9) - 2;

    fmt.font_index = 0;  
    pos.x          = 5;
    page.put_char_at(icons[icon_index], pos, fmt);

    // Show text
    if ((view_mode == 1) || (view_mode == 2)) {
      char str[10];

      if (view_mode == 1) {
        int percentage = (int)((voltage - 3.6f) * 100.0f / (4.2f - 3.6f));
        if (percentage > 100) percentage = 100;
        if (percentage < 0) percentage = 0;
        sprintf(str, "%d%c", percentage, '%');
      }
      else if (view_mode == 2) {
        sprintf(str, "%5.2fv", voltage);
      }

      font = fonts.get(1);
      fmt.font_index = 1;  
      pos.x = 5 + (glyph != nullptr ? glyph->advance : 10) + 5;
      page.put_str_at(str, pos, fmt);
    }
  }

#elif EPUB_INKPLATE_BUILD
  // Original Inkplate implementation
  #include "viewers/page.hpp"
  #include "models/config.hpp"
  #include "battery.hpp"
  #include "screen.hpp"
  #include "logging.hpp"

  #include <cstring>

  static constexpr char const * TAG = "BatteryViewer";

  void
  BatteryViewer::show()
  {
    int8_t view_mode;
    config.get(Config::Ident::BATTERY, &view_mode);

    if (view_mode == 0) return;

    float voltage = battery.read_level();

    LOG_D("Battery voltage: %5.3f", voltage);

    Page::Format fmt = {
      .line_height_factor = 1.0,
      .font_index         = 1,
      .font_size          = 9,
      .indent             = 0,
      .margin_left        = 0,
      .margin_right       = 0,
      .margin_top         = 0,
      .margin_bottom      = 0,
      .screen_left        = 10,
      .screen_right       = 10,
      .screen_top         = 10,
      .screen_bottom      = 10,
      .width              = 0,
      .height             = 0,
      .vertical_align     = 0,
      .trim               = true,
      .pre                = false,
      .font_style         = Fonts::FaceStyle::NORMAL,
      .align              = CSS::Align::LEFT,
      .text_transform     = CSS::TextTransform::NONE,
      .display            = CSS::Display::INLINE
    };

    // Show battery icon

    Font * font = fonts.get(0);

    if (font == nullptr) {
      LOG_E("Internal error (Drawings Font not available!");
      return;
    }

    float   value = ((voltage - 2.5) * 4.0) / 1.2;
    int16_t icon_index =  value; // max is 3.7
    if (icon_index > 4) icon_index = 4;

    static constexpr char icons[5] = { '0', '1', '2', '3', '4' };

    Font::Glyph * glyph = font->get_glyph(icons[icon_index], 9);

    Dim dim;
    dim.width  =  100;
    dim.height = -font->get_descender_height(9);

    Pos pos;
    // pos.x = 4;
    pos.y = Screen::get_height() + font->get_descender_height(9) - 2;

    // page.clear_region(dim, pos);

    fmt.font_index = 0;  
    pos.x          = 5;
    page.put_char_at(icons[icon_index], pos, fmt);

    // LOG_E("Battery icon index: %d (%c)", icon_index, icons[icon_index]);

    // Show text

    if ((view_mode == 1) || (view_mode == 2)) {
      char str[10];

      if (view_mode == 1) {
        int percentage = ((voltage - 2.5) * 100.0) / 1.2;
        if (percentage > 100) percentage = 100;
        sprintf(str, "%d%c", percentage, '%');
      }
      else if (view_mode == 2) {
        sprintf(str, "%5.2fv", voltage);
      }

      font = fonts.get(1);
      fmt.font_index = 1;  
      pos.x = 5 + (glyph != nullptr ? glyph->advance : 10) + 5;
      page.put_str_at(str, pos, fmt);
    }
  }
#endif