#include <stdint.h>
#include <stdio.h>

const uint16_t big_tiles[16] = {
    0x0000, 0x9000, 0x0700, 0x8800,
    0x0030, 0x6060, 0x0730, 0xd860,
    0x0001, 0x9001, 0x0404, 0x8a04,
    0x0022, 0x60b2, 0x042c, 0x5555
};

#define colour_varients 2
const uint8_t colours[colour_varients][4][3] = {
    {
        { 59,  23,  37},
        {115,  23,  45},
        {180,  32,  42},
        {223,  62,  35},
    },
    {
        { 20,  52, 100},
        { 36, 159, 222},
        { 32, 214, 199},
        {166, 252, 219},
    },
};

const uint32_t tile_disp[14] = {
    0x00000000, 0x5050f8fe, 0x5555ffff, 0xf5f5ffff,
    0x50505050, 0x55555555, 0xf5f5f5f5, 0x00005050,
    0x00005555, 0x80e0f5f5, 0x50505555, 0x5555f5f5,
    0x55555751, 0xb5255555
};

const char lightness[4] = {':', '/', 'X', '@'};

#define height 4
#define width 4

#define tile_hight 4
#define tile_width 4

#define img_height (height * 2)
#define img_width (width * 2)

#define display_height (img_height * tile_hight)
#define display_width (img_width * tile_width * 2)

#define shape_count 20

const uint8_t shapes[shape_count][height][width] = {
    {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 1, 1, 1},
    },
    {
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 1},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 0, 0, 1},
        {0, 1, 1, 1},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 1},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 1},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 1},
        {0, 0, 1, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 0, 1, 1},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0},
    },
    {
        {1, 1, 1, 1},
        {1, 0, 1, 1},
        {0, 1, 0, 1},
        {0, 1, 1, 0},
    }
};

uint8_t image[img_height][img_width] = {0};

uint8_t get_shape(int shape, int shp_x, int shp_y) {
    if(shp_x < 0 || shp_x >= width || shp_y < 0 || shp_y >= height) {
        return 0;
    }
    return shapes[shape][shp_y][shp_x];
}

void set_image(int img_x, int img_y, uint8_t val) {
    if(img_x < 0 || img_x >= img_width || img_y < 0 || img_y >= img_height) {
        return;
    }
    image[img_y][img_x] = val;
}

char get_disp_char(int dsp_x, int dsp_y) {
    int tile = image[dsp_y / tile_hight][(dsp_x / 2) / tile_width];
    if(tile == 0)
        return ' ';
    uint32_t disp = tile_disp[tile];
    int index = ((dsp_y % tile_hight) * tile_width) + ((dsp_x / 2) % tile_width);

    char disp_char = lightness[(disp >> (index * 2)) & 0x3];

    return disp_char;
}

void print_shape(int shape) {
    for(int y = 0; y < height + 1; y++) {
        for(int x = 0; x < width + 1; x++) {
            uint8_t big_tile = (
                get_shape(shape, x - 1, y - 1) |
                (get_shape(shape, x, y - 1) << 1) |
                (get_shape(shape, x - 1, y) << 2) |
                (get_shape(shape, x, y) << 3)
            );
            uint16_t mask = big_tiles[big_tile];
            int img_x = x * 2, img_y = y * 2;
            set_image(img_x - 1, img_y - 1, (mask & 0xF000) >> 12);
            set_image(img_x    , img_y - 1, (mask & 0x0F00) >> 8 );
            set_image(img_x - 1, img_y    , (mask & 0x00F0) >> 4 );
            set_image(img_x    , img_y    , (mask & 0x000F) >> 0 );
        }
    }
    printf("+");
    for(int x = 0; x < display_width; x++) {
        printf("-");
    }
    printf("+\n");
    for(int y = 0; y < display_height; y++) {
        printf("|");
        for(int x = 0; x < display_width; x++) {
            printf("%c", get_disp_char(x, y));
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

int main(int argc, char const *argv[]) {
    for(int shape = 0; shape < shape_count; shape++) {
        print_shape(shape);
    }
    return 0;
}
