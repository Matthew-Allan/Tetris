#ifndef AUTOTILE_H
#define AUTOTILE_H

#include <stdint.h>

#include "data.h"

#define tile_hight 4
#define tile_width 4

#define img_height (shp_height * 2)
#define img_width (shp_width * 2)

#define display_height (img_height * tile_hight)
#define display_width (img_width * tile_width * 2)

void get_shape_data(int shape, uint8_t scheme, uint8_t image[img_height][img_width]);
void get_shape_hit(int shape, uint8_t hitbox[shp_height][shp_width]);
void print_shape(uint8_t image[img_height][img_width]);

#endif