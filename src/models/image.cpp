// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#include "models/image.hpp"

#include "alloc.hpp"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_SATURATE_INT

#include "stb_image_resize.h"

void 
Image::resize(Dim new_dim)
{
  LOG_D("Resize to [%d, %d] %d bytes.", new_dim.width, new_dim.height, new_dim.width * new_dim.height);

  if (image_data.bitmap != nullptr) {
    uint8_t * resized_bitmap = (uint8_t *) allocate(new_dim.width * new_dim.height);

    stbir_resize_uint8_generic(
      image_data.bitmap, (int)image_data.dim.width, (int)image_data.dim.height, 0,
      resized_bitmap,    (int)new_dim.width,        (int)new_dim.height,        0,
      1, -1, 0,
      STBIR_EDGE_CLAMP, STBIR_FILTER_CATMULLROM, STBIR_COLORSPACE_LINEAR,
      nullptr);

    free(image_data.bitmap);

    image_data.bitmap = resized_bitmap;
    image_data.dim    = new_dim; 
  }
}
