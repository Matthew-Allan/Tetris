#include "autotile.h"

#include <stdio.h>

const char lightness[4] = {':', '/', 'X', '@'};

uint8_t get_shape(int shape, int shp_x, int shp_y) {
    if(shp_x < 0 || shp_x >= shp_width || shp_y < 0 || shp_y >= shp_height) {
        return 0;
    }
    int index = shp_x + (shp_y * shp_width);
    return (shapes[shape] >> index) & 1;
}

void set_image(int img_x, int img_y, uint8_t val, uint8_t image[img_height][img_width]) {
    if(img_x < 0 || img_x >= img_width || img_y < 0 || img_y >= img_height) {
        return;
    }
    image[img_y][img_x] = val;
}

char get_disp_char(int dsp_x, int dsp_y, uint8_t image[img_height][img_width]) {
    int tile = image[dsp_y / tile_hight][(dsp_x / 2) / tile_width] & 0xf;
    if(tile == 0)
        return ' ';
    uint32_t disp = tile_disp[tile];
    int index = ((dsp_y % tile_hight) * tile_width) + ((dsp_x / 2) % tile_width);

    char disp_char = lightness[(disp >> (index * 2)) & 0x3];

    return disp_char;
}

void populate_image(int shape, uint8_t scheme, uint8_t image[img_height][img_width]) {
    for(int y = 0; y < shp_height + 1; y++) {
        for(int x = 0; x < shp_width + 1; x++) {
            uint8_t big_tile = (
                get_shape(shape, x - 1, y - 1) |
                (get_shape(shape, x, y - 1) << 1) |
                (get_shape(shape, x - 1, y) << 2) |
                (get_shape(shape, x, y) << 3)
            );
            uint8_t scheme_part = scheme << 4;
            uint16_t mask = big_tiles[big_tile];
            int img_x = x * 2, img_y = y * 2;
            set_image(img_x - 1, img_y - 1, scheme_part | ((mask & 0xF000) >> 12), image);
            set_image(img_x    , img_y - 1, scheme_part | ((mask & 0x0F00) >> 8 ), image);
            set_image(img_x - 1, img_y    , scheme_part | ((mask & 0x00F0) >> 4 ), image);
            set_image(img_x    , img_y    , scheme_part | ((mask & 0x000F) >> 0 ), image);
        }
    }
}

void print_shape(uint8_t image[img_height][img_width]) {
    printf("+");
    for(int x = 0; x < display_width; x++) {
        printf("-");
    }
    printf("+\n");
    for(int y = 0; y < display_height; y++) {
        printf("|");
        for(int x = 0; x < display_width; x++) {
            printf("%c", get_disp_char(x, y, image));
        }
        printf("|\n");
    }
    printf("+");
    for(int x = 0; x < display_width; x++) {
        printf("-");
    }
    printf("+\n");
    printf("\n");
}

void get_shape_data(int shape, uint8_t scheme, uint8_t image[img_height][img_width]) {
    populate_image(shape, scheme, image);
}

void get_shape_hit(int shape, uint8_t hitbox[shp_height][shp_width]) {
    for(int y = 0; y < shp_height; y++) {
        for(int x = 0; x < shp_width; x++) {
            hitbox[y][x] = get_shape(shape, x, y);
        }
    }
}
