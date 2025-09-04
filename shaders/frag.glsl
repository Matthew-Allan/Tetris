#version 330 core
in vec2 f_tex_pos;

out vec4 col;

uniform usampler1D tex;
uniform sampler2D colours;
uniform usampler2D shape;
uniform usampler2D grid;

uniform ivec2 shape_pos;

const int tile_size = 4;

const ivec2 dim = ivec2(20, 40);
const vec3 background = vec3(25, 25, 25) / 255;

vec4 get_tile_pixel(ivec2 pos, uint tile, uint scheme) {
    if(tile == 0u || tile > 14u) {
        return vec4(background, 1);
    }
    uint tile_data = texelFetch(tex, int(tile), 0).r;
    int pixel_index = pos.x + (pos.y * tile_size);
    uint colour_index = tile_data >> (pixel_index * 2) & 3u;
    return texelFetch(colours, ivec2(colour_index, scheme), 0);
}

void main() {
    vec2 flipped_pos = vec2(f_tex_pos.x, 1 - f_tex_pos.y);
    vec2 screen = vec2(flipped_pos * dim);
    ivec2 pixel_coords = ivec2(mod(screen * tile_size, tile_size));
    
    uint tile_data = texelFetch(grid, ivec2(screen), 0).r;
    uint tile = tile_data & 0xFu;
    if(tile == 0u) {
        tile_data = texelFetch(shape, ivec2(screen) - (2 * shape_pos), 0).r;
        tile = tile_data & 0xFu;
    }
    uint scheme = (tile_data >> 4) & 0xFu;

    col = get_tile_pixel(pixel_coords, tile, scheme);
}