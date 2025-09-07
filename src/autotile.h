#ifndef AUTOTILE_H
#define AUTOTILE_H

#include <stdint.h>

#include "data.h"

#define IMG_HEIGHT (SHP_HEIGHT * 2) // Height of an image buffer required to store a shape.
#define IMG_WIDTH (SHP_WIDTH * 2)   // Width of an image buffer required to store a shape.

#define TILE_HEIGHT 4   // Height of a tile in pixels.
#define TILE_WIDTH 4    // Width of a tile in pixels.

typedef uint8_t imageBuffer[IMG_HEIGHT][IMG_WIDTH];
typedef uint8_t hitboxBuffer[SHP_HEIGHT][SHP_WIDTH];

void getShapeData(int shape, uint8_t scheme, imageBuffer image);
void getShapeHit(int shape, hitboxBuffer hitbox);
void printShape(imageBuffer image);

#endif