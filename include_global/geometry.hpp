// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#pragma once

#include <cstdint>

struct Dim { 
  uint16_t width; 
  uint16_t height; 
  Dim(uint16_t w, uint16_t h) { 
    width  = w; 
    height = h;
  }
  Dim() {}
};

struct Pos { 
  uint16_t x; 
  uint16_t y; 
  Pos(uint16_t xpos, uint16_t ypos) { 
    x = xpos; 
    y = ypos; 
  }
  Pos() {}
};

struct Dim8 { 
  uint8_t width; 
  uint8_t height; 
  Dim8(uint8_t w, uint8_t h) { 
    width  = w; 
    height = h;
  }
  Dim8() {}
};

struct Pos8 { 
  uint8_t x; 
  uint8_t y; 
  Pos8(uint8_t xpos, uint8_t ypos) { 
    x = xpos; 
    y = ypos; 
  }
  Pos8() {}
};
