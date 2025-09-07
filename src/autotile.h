#ifndef AUTOTILE_H
#define AUTOTILE_H

#include <stdint.h>

#include "data.h"

#define TILE_HEIGHT 4
#define TILE_WIDTH 4

#define IMG_HEIGHT (SHP_HEIGHT * 2)
#define IMG_WIDTH (SHP_WIDTH * 2)

#define DISPLAY_HEIGHT (IMG_HEIGHT * TILE_HEIGHT)
#define DISPLAY_WIDTH (IMG_WIDTH * TILE_WIDTH * 2)

void getShapeData(int shape, uint8_t scheme, uint8_t image[IMG_HEIGHT][IMG_WIDTH]);
void getShapeHit(int shape, uint8_t hitbox[SHP_HEIGHT][SHP_WIDTH]);
void printShape(uint8_t image[IMG_HEIGHT][IMG_WIDTH]);

#endif