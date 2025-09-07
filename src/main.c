#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "shader.h"
#include "data.h"
#include "autotile.h"
#include "paths.h"

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600

#define GL_MAJOR_VER 3
#define GL_MINOR_VER 3

#define WINDOW_FLAGS SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI

#define fps 60

#define GRID_WIDTH 10
#define GRID_HEIGHT 20

#define S_GRID_WIDTH (GRID_WIDTH * 2)
#define S_GRID_HEIGHT (GRID_HEIGHT * 2)

#define DROP_TIME 300

uint8_t tile_grid[S_GRID_HEIGHT][S_GRID_WIDTH] = {0};
uint8_t grid[GRID_HEIGHT][GRID_WIDTH] = {0};
int grid_updated = 1;

typedef struct shape_bag {
    uint8_t bag_size;
    uint8_t bag[shape_types];
} shape_bag;

typedef struct falling_shape {
    GLuint texture;
    int drop_timer;
    int shape;
    int rot;
    int y;
    int x;
    uint8_t data[img_height][img_width];
    uint8_t hitbox[shp_height][shp_width];
    uint8_t scheme : 4;
    uint8_t updated : 1;
    uint8_t placed : 1;
} falling_shape;

SDL_Window *create_window() {
    printf("Initing SDL.\n");
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not init SDL: %s\n", SDL_GetError());
        return NULL;
    }

    // Set profile to core and version.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_MAJOR_VER);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_MINOR_VER);

    // Create the SDL window.
    printf("Creating window.\n");
    SDL_Window *window = SDL_CreateWindow(
        "Tetris",                                       // Window title
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, // Center the x and y position on the screen.
        SCREEN_WIDTH, SCREEN_HEIGHT,                    // Set height and width.
        WINDOW_FLAGS                                    // (Flags) Open window useable with OpenGL context.
    );

    // Check that the window was created.
    if(!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return NULL;
    }

    // Create a GL context.
    printf("Creating context.\n");
    SDL_GL_CreateContext(window);

    // Give glad the function loader specific to the OS.
    printf("Loading GL loader.\n");
    if(!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        printf("Failed to load GL\n");
        return NULL;
    }

    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    glViewport(0, 0, height, width);
    glClearColor(0, 0, 0, 1.0f);

    return window;
}

void setup_screen_vao() {
    float verticies[] = {
         0.5,  1,  0,   1, 1,   // top right
         0.5, -1,  0,   1, 0,   // bottom right
        -0.5, -1,  0,   0, 0,   // bottom left
        -0.5,  1,  0,   0, 1    // top left 
    };

    unsigned int indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };


    GLuint VAO, VBO, EBO;

    // Generate a vertex array object.
    glGenVertexArrays(1, &VAO);

    // Bind the vertex array object.
    glBindVertexArray(VAO);

    // Generate buffer objects.
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the virtual and element buffer objects.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Tell the shader how to interpret the VBO.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}

GLuint setup_shader_data(GLuint shader) {
    GLuint textures, colours;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &textures);
    glBindTexture(GL_TEXTURE_1D, textures);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, tile_types, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, tile_disp);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &colours);
    glBindTexture(GL_TEXTURE_2D, colours);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, scheme_size, colour_varients, 0, GL_RGB, GL_UNSIGNED_BYTE, colour_schemes);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "tex"), 0);
    glUniform1i(glGetUniformLocation(shader, "colours"), 1);
    glUniform1i(glGetUniformLocation(shader, "shape"), 2);
    glUniform1i(glGetUniformLocation(shader, "grid"), 3);

    return glGetUniformLocation(shader, "shape_pos");
}

void update_shape_tex(falling_shape *shape) {
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, shape->texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0, 
        GL_R8UI, 8, 8, 
        0,
        GL_RED_INTEGER, GL_UNSIGNED_BYTE,
        shape->data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void update_grid_tex(GLuint grid_tex) {
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, grid_tex);

    glTexImage2D(
        GL_TEXTURE_2D,
        0, 
        GL_R8UI, S_GRID_WIDTH, S_GRID_HEIGHT, 
        0,
        GL_RED_INTEGER, GL_UNSIGNED_BYTE,
        tile_grid
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void shift_rows_down(int y, int rows) {
    int blocks_shifted = 1;
    y -= rows;
    for(; blocks_shifted && y >= 0; y--) {
        blocks_shifted = 0;
        for(int x = 0; x < GRID_WIDTH; x++) {
            blocks_shifted |= grid[y][x] || grid[y + rows][x] ^ grid[y][x];;
            grid[y + rows][x] = grid[y][x];
            for(int t_y = 0; t_y < 2; t_y++) {
                for(int t_x = 0; t_x < 2; t_x++) {
                    tile_grid[((y + rows) * 2) + t_y][(x * 2) + t_x] = tile_grid[(y * 2) + t_y][(x * 2) + t_x];
                }
            }
        }
    }
}

void place_shape(falling_shape *shape) {
    int clear[4] = {0};
    for(int s_y = 0; s_y < shp_height; s_y++) {
        int y = shape->y + s_y;
        for(int s_x = 0; s_x < shp_width; s_x++) {
            if(!shape->hitbox[s_y][s_x]) {
                continue;
            }
            int x = shape->x + s_x;
            grid[y][x] = 1;
            for(int t_y = 0; t_y < 2; t_y++) {
                for(int t_x = 0; t_x < 2; t_x++) {
                    tile_grid[(y * 2) + t_y][(x * 2) + t_x] = shape->data[(s_y * 2) + t_y][(s_x * 2) + t_x];
                }
            }
        }
        clear[s_y] = 1;
        for(int x = 0; clear[s_y] && x < GRID_WIDTH; x++) {
            clear[s_y] = grid[y][x];
        }
    }

    int clear_streak = 0;
    for(int s_y = 0; s_y < shp_height; s_y++) {
        int y = shape->y + s_y;
        clear_streak += clear[s_y];
        if((!clear[s_y] && clear_streak)) {
            shift_rows_down(y - 1, clear_streak);
            clear_streak = 0;
        } else if (s_y == shp_height - 1 && clear_streak) {
            shift_rows_down(y, clear_streak);
        }
    }
    grid_updated = 1;
    shape->placed = 1;
}

int shape_index(int shape, int rot) {
    return (shape * shape_rotations) + rot;
}

void update_shape(falling_shape *shape) {
    int index = shape_index(shape->shape, shape->rot);
    get_shape_data(index, shape->scheme, shape->data);
    get_shape_hit(index, shape->hitbox);
    shape->updated = 1;
}

int check_valid(falling_shape *shape, int x, int y, int rot) {
    uint8_t test_hitbox[shp_height][shp_width];
    get_shape_hit(shape_index(shape->shape, rot), test_hitbox);
    for(int s_y = 0; s_y < shp_height; s_y++) {
        for(int s_x = 0; s_x < shp_width; s_x++) {
            if(!test_hitbox[s_y][s_x]) {
                continue;
            }

            int blk_y = s_y + y, blk_x = s_x + x;
            if(blk_y >= GRID_HEIGHT || blk_x >= GRID_WIDTH || blk_x < 0 || grid[blk_y][blk_x]) {
                return 0;
            }
        }
    }
    return 1;
}

int move_shape(falling_shape *shape, int x, int y) {
    x += shape->x; y += shape->y;
    if(!check_valid(shape, x, y, shape->rot)) {
        return 0;
    }
    shape->x = x; shape->y = y;
    update_shape(shape);
    return 1;
}

int rotate_shape(falling_shape *shape, int rot) {
    rot = (rot + shape->rot) % shape_rotations;
    if(rot < 0) {
        rot += shape_rotations;
    }
    if(!check_valid(shape, shape->x, shape->y, rot)) {
        return 0;
    }
    shape->rot = rot;
    update_shape(shape);
    return 1;
}

void fill_bag(shape_bag *bag) {
    for(int i = 0; i < shape_types; i++) {
        bag->bag[i] = 0;
    }
    for(int i = shape_types; i > 0; i--) {
        int index = rand() % i;
        for(int j = 0; j <= index; j++) {
            while(bag->bag[j]) {
                j++; index++;
            }
        }
        bag->bag[index] = i - 1;
    }
    bag->bag_size = shape_types;
}

int select_shape(shape_bag *bag) {
    if(bag->bag_size == 0) {
        fill_bag(bag);
    }
    bag->bag_size--;
    return bag->bag[bag->bag_size];
}

void reset_shape(falling_shape *shape, shape_bag *bag) {
    shape->shape = select_shape(bag);
    shape->scheme = rand() % colour_varients;
    shape->rot = 0;
    shape->y = 0;
    shape->x = 3;
    shape->drop_timer = DROP_TIME;
    shape->updated = 1;
    shape->placed = 0;
    update_shape(shape);
}

void gravity(falling_shape *shape) {
    if(!move_shape(shape, 0, 1)) {
        place_shape(shape);
    }
}

void drop_shape(falling_shape *shape) {
    while(!shape->placed) {
        gravity(shape);
    }
}

int poll_events(falling_shape *shape) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return 0;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q: rotate_shape(shape, 1);
                break;
            case SDLK_e: rotate_shape(shape, -1);
                break;
            case SDLK_d: move_shape(shape, 1, 0);
                break;
            case SDLK_a: move_shape(shape, -1, 0);
                break;
            case SDLK_SPACE: drop_shape(shape);
                break;
            default:
                break;
            }
            break;   
        }
    }
    return 1;
}

int run_game(SDL_Window *window) {
    GLuint shader;
    char *vert_path = get_path("shaders/vert.glsl"), *frag_path = get_path("shaders/frag.glsl");
    int shader_outcome = buildShader(&shader, vert_path, frag_path);
    free(vert_path);
    free(frag_path);
    
    if(shader_outcome == -1) {
        return -1;
    }

    setup_screen_vao();
    GLuint pos_loc = setup_shader_data(shader);
    
    GLuint grid_tex;
    glGenTextures(1, &grid_tex);

    shape_bag bag;
    fill_bag(&bag);

    falling_shape shape;
    glGenTextures(1, &shape.texture);
    srand(time(NULL));
    reset_shape(&shape, &bag);

    int running = 1;
    uint64_t prev_time = SDL_GetTicks64();

    while(running) {
        uint64_t time_value;
        while(
            (running = poll_events(&shape)) &&
            (time_value = SDL_GetTicks64()) - prev_time < (1000 / fps)
        ) {
            SDL_Delay(1);
        }
        uint64_t delta_time = time_value - prev_time;
        prev_time = time_value;

        shape.drop_timer -= delta_time;
        while(shape.drop_timer <= 0) {
            shape.drop_timer += DROP_TIME;
            gravity(&shape);
        }

        if(shape.placed) {
            reset_shape(&shape, &bag);
            if(!check_valid(&shape, shape.x, shape.y, shape.rot)) {
                return 0;
            }
        }

        if(shape.updated) {
            update_shape_tex(&shape);
        }

        if(grid_updated) {
            update_grid_tex(grid_tex);
            grid_updated = 0;
        }

        glUniform2i(pos_loc, shape.x, shape.y);

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}

int main(int argc, char const *argv[]) {
    SDL_Window *window = create_window();
    if(window) {
        run_game(window);
    }

    SDL_Quit();
    return 0;
}
