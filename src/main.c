#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "shader.h"
#include "data.h"
#include "autotile.h"

#define SCREEN_WIDTH 600    // Width of the screen in window points (not in pixels).
#define SCREEN_HEIGHT 600   // Height of the screen in window points (not in pixels).

#define GL_MAJOR_VER 3  // The OpenGL major version number to target.
#define GL_MINOR_VER 3  // The OpenGL minor version number to target.

// Flags for when creating a window. Use OpenGL and allow high DPI.
#define WINDOW_FLAGS SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI

// Set the target FPS.
#define fps 60

#define GRID_HEIGHT 20  // Height of the game board.
#define GRID_WIDTH 10   // Width of the game board.

#define S_GRID_HEIGHT (GRID_HEIGHT * 2) // Height of the game board in small tiles.
#define S_GRID_WIDTH (GRID_WIDTH * 2)   // Width of the game board in small tiles.

// How long it takes for a block to drop in ms.
#define DROP_TIME 300

// The grid of indicies for which tiles to display.
uint8_t tile_grid[S_GRID_HEIGHT][S_GRID_WIDTH] = {0};

// The grid showing where blocks have already fallen to be checked agains the falling blocks hitbox.
uint8_t grid[GRID_HEIGHT][GRID_WIDTH] = {0};

// If the grid has been updated and needs to have its texture updated.
int grid_updated = 1;

// A struct for storing a bag of random shapes.
typedef struct shapeBag {
    uint8_t bag_size;           // Number of shapes left in the bag.
    uint8_t bag[SHAPE_TYPES];   // Array of shape indices in a random order.
} shapeBag;

// A struct for holding any info a falling shape might need.
typedef struct fallingShape {
    GLuint texture;         // The texture identifier for the shape.
    GLuint pos_loc;         // The location of the shape pos uniform.
    int drop_timer;         // How long until the next drop.
    int shape;              // Which shape is selected.
    int rot;                // The rotation of the shape.
    int y;                  // The y position of the shape.
    int x;                  // The x position of the shape.
    imageBuffer image;      // The autotiled shape data to be sent to the GPU.
    hitboxBuffer hitbox;    // The hitbox of the shape.
    uint8_t scheme : 4;     // The colour scheme of the shape.
    uint8_t updated : 1;    // If the shape has been updated and needs to have its texture updated.
    uint8_t moved : 1;      // If the shape has been moved and needs to have position updated.
    uint8_t placed : 1;     // If the shape has been placed and needs resetting.
} fallingShape;

// Create an SDL window and an OpenGL context.
SDL_Window *createWindow() {
    // Init SDL.
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

    // Set viewport to the correct width and height.
    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    glViewport(0, 0, height, width);

    // Set clear colour to black.
    glClearColor(0, 0, 0, 1.0f);

    // Return a pointer to the window.
    return window;
}

// Set up a vertex array object that the game board should be rendered on.
void setupScreenVAO() {
    float verticies[] = {
        // Vertex pos   UV pos
         0.5,  1,  0,   1, 1,   // top right
         0.5, -1,  0,   1, 0,   // bottom right
        -0.5, -1,  0,   0, 0,   // bottom left
        -0.5,  1,  0,   0, 1    // top left 
    };

    unsigned int indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    // Generate a vertex array object.
    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    // Bind the vertex array object.
    glBindVertexArray(VAO);

    // Generate buffer objects.
    GLuint VBO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the virtual and element buffer objects.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Send the vertex and index data to the buffer objects.
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Tell the shader how to interpret the VBO.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

    // Enable the attributes.
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}

// Send the tile and colour data to the shader.
void setupShaderData(GLuint shader) {
    // Send the texture data.
    GLuint textures;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &textures);
    glBindTexture(GL_TEXTURE_1D, textures);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, TILE_TYPES, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, TILE_DISP);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Send the colour data.
    GLuint colours;
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &colours);
    glBindTexture(GL_TEXTURE_2D, colours);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCHEME_SIZE, SCHEME_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, COLOUR_SCHEMES);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Tell OpenGL which textures are being used by each variable.
    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "tex"), 0);
    glUniform1i(glGetUniformLocation(shader, "colours"), 1);
    glUniform1i(glGetUniformLocation(shader, "shape"), 2);
    glUniform1i(glGetUniformLocation(shader, "grid"), 3);
}

// Update the shapes texture to the data stored in shape.
void updateShapeTex(fallingShape *shape) {
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, shape->texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0, 
        GL_R8UI, 8, 8, 
        0,
        GL_RED_INTEGER, GL_UNSIGNED_BYTE,
        shape->image
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

// Update the grid texture with the most recent tile grid.
void updateGridTex(GLuint grid_tex) {
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

// Move rows starting at y a certain number of rows down.
void shiftRowsDown(int y, int rows) {
    y -= rows;
    // Loop until no blocks were shifted or it reaches the top.
    for(int blocks_shifted = 1; blocks_shifted && y >= 0; y--) {
        blocks_shifted = 0;

        // Scan along the width of the board.
        for(int x = 0; x < GRID_WIDTH; x++) {
            // Check if it's shifting anything or setting a square to nothing.
            blocks_shifted |= grid[y][x] || grid[y + rows][x] ^ grid[y][x];;

            // Shift hitbox down.
            grid[y + rows][x] = grid[y][x];

            // Shift textures down.
            for(int t_y = 0; t_y < 2; t_y++) {
                for(int t_x = 0; t_x < 2; t_x++) {
                    tile_grid[((y + rows) * 2) + t_y][(x * 2) + t_x] = tile_grid[(y * 2) + t_y][(x * 2) + t_x];
                }
            }
        }
    }
}

// Place a falling shape onto the grid.
void placeShape(fallingShape *shape) {
    // An array that keeps track of if a row in the shape clears a row.
    int clear[4];

    // Loop through the shape y axis.
    for(int s_y = 0; s_y < SHP_HEIGHT; s_y++) {

        // Get the y coord in grid-space.
        int y = shape->y + s_y;

        // Loop through the shape x axis.
        for(int s_x = 0; s_x < SHP_WIDTH; s_x++) {

            if(!shape->hitbox[s_y][s_x]) {
                continue;
            }

            // Get the x coord in grid-space.
            int x = shape->x + s_x;

            // Place hitbox.
            grid[y][x] = 1;

            // Place tile
            for(int t_y = 0; t_y < 2; t_y++) {
                for(int t_x = 0; t_x < 2; t_x++) {
                    tile_grid[(y * 2) + t_y][(x * 2) + t_x] = shape->image[(s_y * 2) + t_y][(s_x * 2) + t_x];
                }
            }
        }

        // Check if the row is now cleared.
        clear[s_y] = 1;
        for(int x = 0; clear[s_y] && x < GRID_WIDTH; x++) {
            clear[s_y] = grid[y][x];
        }
    }

    // Clear and shift rows, checking for any streaks for efficiency.
    int clear_streak = 0;
    for(int s_y = 0; s_y < SHP_HEIGHT; s_y++) {
        clear_streak += clear[s_y];

        // Check if a streak has ended or if its at the end of the shape and shift the rows.
        if(clear_streak && (!clear[s_y] || s_y == SHP_HEIGHT - 1)) {
            int y = shape->y + s_y - !clear[s_y];
            shiftRowsDown(y, clear_streak);
            clear_streak = 0;
        }
    }

    // Tell the main loop that the grids tex needs updating.
    grid_updated = 1;

    // Tell the main loop that the shape needs resetting.
    shape->placed = 1;
}

// Get the index of a shape given its rotations.
int shapeIndex(int shape, int rot) {
    return (shape * SHAPE_ROTATIONS) + rot;
}

// Update the shapes image and hitbox buffers.
void updateShape(fallingShape *shape) {
    int index = shapeIndex(shape->shape, shape->rot);
    getShapeData(index, shape->scheme, shape->image);
    getShapeHit(index, shape->hitbox);
    shape->updated = 1;
}

// Check if a hitbox is allowed to be at the given location.
int checkValidHitbox(hitboxBuffer test_hitbox, int x, int y) {
    for(int s_y = 0; s_y < SHP_HEIGHT; s_y++) {
        for(int s_x = 0; s_x < SHP_WIDTH; s_x++) {
            // If the hitbox isn't set here, no further checks are needed.
            if(!test_hitbox[s_y][s_x]) {
                continue;
            }

            // Test if its in bounds and also if the grid contains a block there.
            int grd_y = s_y + y, grd_x = s_x + x;
            if(grd_y >= GRID_HEIGHT || grd_x >= GRID_WIDTH || grd_x < 0 || grid[grd_y][grd_x]) {
                return 0;
            }
        }
    }

    // No problems found so return 1.
    return 1;
}

// Check if a shape is valid with a given location or rotation.
int checkValid(fallingShape *shape, int x, int y, int rot) {
    // If the shape hasn't rotated, don't need to create a new hitbox.
    if(shape->rot == rot) {
        return checkValidHitbox(shape->hitbox, x, y);
    }

    // Create a new hitbox and check if it's valid at the location.
    hitboxBuffer test_hitbox;
    getShapeHit(shapeIndex(shape->shape, rot), test_hitbox);
    return checkValidHitbox(test_hitbox, x, y);
}

// Attempt to move the shape to a new location.
int moveShape(fallingShape *shape, int x, int y) {
    x += shape->x; y += shape->y;
    if(!checkValid(shape, x, y, shape->rot)) {
        return 0;
    }
    shape->x = x; shape->y = y;
    shape->moved = 1;
    return 1;
}

// Attempt to give the shape a new roation.
int rotateShape(fallingShape *shape, int rot) {
    rot = (rot + shape->rot) % SHAPE_ROTATIONS;
    if(rot < 0) {
        rot += SHAPE_ROTATIONS;
    }
    if(!checkValid(shape, shape->x, shape->y, rot)) {
        return 0;
    }
    shape->rot = rot;
    updateShape(shape);
    return 1;
}

// Randomly fill a bag with shape indicies.
void fillBag(shapeBag *bag) {
    // Clear the bag.
    memset(bag->bag, 0, sizeof(bag->bag));

    // Place one of each index from 0 - SHAPE_TYPES into the bag in reverse order so 0 is placed last.
    for(int i = SHAPE_TYPES; i > 0; i--) {
        // Where to place in the bag. Options are more limited the more items are put in the bag.
        int index = rand() % i;

        // Find the index in the bag skipping any already placed items.
        for(int j = 0; j <= index; j++) {
            while(bag->bag[j]) {
                j++; index++;
            }
        }
        bag->bag[index] = i - 1;
    }
    bag->bag_size = SHAPE_TYPES;
}

// Choose a new shape to use.
int selectShape(shapeBag *bag) {
    if(bag->bag_size == 0) {
        fillBag(bag);
    }
    bag->bag_size--;
    return bag->bag[bag->bag_size];
}

// Reset the shape to the top with a new random colour and shape.
void resetShape(fallingShape *shape, shapeBag *bag) {
    shape->shape = selectShape(bag);
    shape->scheme = rand() % SCHEME_SIZE;
    shape->rot = 0;
    shape->y = 0;
    shape->x = 3;
    shape->drop_timer = DROP_TIME;
    shape->updated = 1;
    shape->moved = 1;
    shape->placed = 0;
    updateShape(shape);
}

// Move the shape down one. Places the shape if it's not possible.
void gravity(fallingShape *shape) {
    if(!moveShape(shape, 0, 1)) {
        placeShape(shape);
    }
}

// Drop the shape until it's placed.
void dropShape(fallingShape *shape) {
    while(!shape->placed) {
        gravity(shape);
    }
}

// Check for any SDL events. Handles quitting and button presses. Returns if game should keep running.
int pollEvents(fallingShape *shape) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return 0;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q: rotateShape(shape, 1);
                break;
            case SDLK_e: rotateShape(shape, -1);
                break;
            case SDLK_d: moveShape(shape, 1, 0);
                break;
            case SDLK_a: moveShape(shape, -1, 0);
                break;
            case SDLK_SPACE: dropShape(shape);
                break;
            default:
                break;
            }
            break;   
        }
    }
    return 1;
}

// Run the game and gameloop in the given window.
int runGame(SDL_Window *window) {
    // Compile the shaders into a program.
    GLuint shader;
    if(buildShader(&shader, "shaders/vert.glsl", "shaders/frag.glsl") == -1) {
        return -1;
    }

    // Create a vertex object that covers the screen where the game is displayed.
    setupScreenVAO();
    setupShaderData(shader);
    
    // Create a texture for the grid to be on.
    GLuint grid_tex;
    glGenTextures(1, &grid_tex);

    // Create a shape bag for randomly choosing the next shape.
    shapeBag bag;
    srand(time(NULL));
    fillBag(&bag);

    // Create the shape that will fall from the screen.
    fallingShape shape;
    glGenTextures(1, &shape.texture);
    shape.pos_loc = glGetUniformLocation(shader, "shape_pos");
    resetShape(&shape, &bag);

    // Set the time in ms that the game starts.
    uint64_t prev_time = SDL_GetTicks64();

    // Main game loop
    int running = 1;
    while(running) {
        // Loop until enough time has passed for the fps to be correct.
        uint64_t time_value;
        while((time_value = SDL_GetTicks64()) - prev_time < (1000 / fps)) {
            if(!(running = pollEvents(&shape))) {
                break;
            }
            SDL_Delay(1);
        }
        // Work out time between last frame and this one.
        uint64_t delta_time = time_value - prev_time;
        prev_time = time_value;

        // Check if timer tells the block to drop down.
        shape.drop_timer -= delta_time;
        while(shape.drop_timer <= 0) {
            shape.drop_timer += DROP_TIME;
            gravity(&shape);
        }

        // Check if the shape has been placed. If so reset the shape and make sure the game hasn't ended.
        if(shape.placed) {
            resetShape(&shape, &bag);
            if(!checkValid(&shape, shape.x, shape.y, shape.rot)) {
                return 0;
            }
        }

        // The following checks happen here so it doesn't get called multiple times per frame.
        // Check if the shapes texture or position needs to be updated.
        if(shape.updated) {
            updateShapeTex(&shape);
        }
        if(shape.moved) {
            glUniform2i(shape.pos_loc, shape.x, shape.y);
        }

        // Check if the grids texture needs to be updated.
        if(grid_updated) {
            updateGridTex(grid_tex);
            grid_updated = 0;
        }

        // Clear the screen and draw the grid to the screen.
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap the buffers.
        SDL_GL_SwapWindow(window);
    }

    return 0;
}

int main(int argc, char const *argv[]) {
    // Create the window
    SDL_Window *window = createWindow();

    // If window created, run the game.
    if(window) {
        runGame(window);
    }

    // Quit and return.
    SDL_Quit();
    return 0;
}
