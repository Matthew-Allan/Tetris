#include "autotile.h"

#include <stdio.h>

const char GRADIENT[4] = {':', '/', 'X', '@'};

uint8_t getShape(int shape, int shp_x, int shp_y) {
    if(shp_x < 0 || shp_x >= SHP_WIDTH || shp_y < 0 || shp_y >= SHP_HEIGHT) {
        return 0;
    }
    int index = shp_x + (shp_y * SHP_WIDTH);
    return (SHAPES[shape] >> index) & 1;
}

void setImage(int img_x, int img_y, uint8_t val, uint8_t image[IMG_HEIGHT][IMG_WIDTH]) {
    if(img_x < 0 || img_x >= IMG_WIDTH || img_y < 0 || img_y >= IMG_HEIGHT) {
        return;
    }
    image[img_y][img_x] = val;
}

char getDispChar(int dsp_x, int dsp_y, uint8_t image[IMG_HEIGHT][IMG_WIDTH]) {
    int tile = image[dsp_y / TILE_HEIGHT][(dsp_x / 2) / TILE_WIDTH] & 0xf;
    if(tile == 0)
        return ' ';
    uint32_t disp = TILE_DISP[tile];
    int index = ((dsp_y % TILE_HEIGHT) * TILE_WIDTH) + ((dsp_x / 2) % TILE_WIDTH);

    char disp_char = GRADIENT[(disp >> (index * 2)) & 0x3];

    return disp_char;
}

void populateImage(int shape, uint8_t scheme, uint8_t image[IMG_HEIGHT][IMG_WIDTH]) {
    for(int y = 0; y < SHP_HEIGHT + 1; y++) {
        for(int x = 0; x < SHP_WIDTH + 1; x++) {
            uint8_t big_tile = (
                getShape(shape, x - 1, y - 1) |
                (getShape(shape, x, y - 1) << 1) |
                (getShape(shape, x - 1, y) << 2) |
                (getShape(shape, x, y) << 3)
            );
            uint8_t scheme_part = scheme << 4;
            uint16_t mask = BIG_TILES[big_tile];
            int img_x = x * 2, img_y = y * 2;
            setImage(img_x - 1, img_y - 1, scheme_part | ((mask & 0xF000) >> 12), image);
            setImage(img_x    , img_y - 1, scheme_part | ((mask & 0x0F00) >> 8 ), image);
            setImage(img_x - 1, img_y    , scheme_part | ((mask & 0x00F0) >> 4 ), image);
            setImage(img_x    , img_y    , scheme_part | ((mask & 0x000F) >> 0 ), image);
        }
    }
}

void printShape(uint8_t image[IMG_HEIGHT][IMG_WIDTH]) {
    printf("+");
    for(int x = 0; x < DISPLAY_WIDTH; x++) {
        printf("-");
    }
    printf("+\n");
    for(int y = 0; y < DISPLAY_HEIGHT; y++) {
        printf("|");
        for(int x = 0; x < DISPLAY_WIDTH; x++) {
            printf("%c", getDispChar(x, y, image));
        }
        printf("|\n");
    }
    printf("+");
    for(int x = 0; x < DISPLAY_WIDTH; x++) {
        printf("-");
    }
    printf("+\n");
    printf("\n");
}

void getShapeData(int shape, uint8_t scheme, uint8_t image[IMG_HEIGHT][IMG_WIDTH]) {
    populateImage(shape, scheme, image);
}

void getShapeHit(int shape, uint8_t hitbox[SHP_HEIGHT][SHP_WIDTH]) {
    for(int y = 0; y < SHP_HEIGHT; y++) {
        for(int x = 0; x < SHP_WIDTH; x++) {
            hitbox[y][x] = getShape(shape, x, y);
        }
    }
}
