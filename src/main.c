#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600

#define fps 60

int main(int argc, char const *argv[]) {
    printf("Initing SDL.\n");
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not init SDL: %s\n", SDL_GetError());
        return -1;
    }

    printf("Initing SDL image.\n");
    if(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) < 0) {
        printf("Could not init SDL image: %s\n", IMG_GetError());
        return -1;
    }

    // Set profile to core and version to 3.3.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // Create the SDL window.
    printf("Creating window.\n");
    SDL_Window *window = SDL_CreateWindow(
        "Tetris",                                       // Window title
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, // Center the x and y position on the screen.
        SCREEN_WIDTH, SCREEN_HEIGHT,                    // Set height and width.
        SDL_WINDOW_OPENGL                               // (Flags) Open window useable with OpenGL context.
    );

    // Check that the window was created.
    if(!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return -1;
    }

    // Create a GL context.
    printf("Creating context.\n");
    SDL_GL_CreateContext(window);

    // Give glad the function loader specific to the OS.
    printf("Loading GL loader.\n");
    if(!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        printf("Failed to load GL: %s\n", SDL_GetError());
        return -1;
    }

    // Set the viewport to the same height and width as the window.
    glViewport(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    uint64_t prev_time = 0;

    int running = 1;

    while(running) {
        
        uint64_t time_value = SDL_GetTicks64();
        while(time_value - prev_time < (1000 / fps)) {
            SDL_Delay(1);
            
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                }
            }

            time_value = SDL_GetTicks64();
        }
        uint64_t delta_time = time_value - prev_time;
        prev_time = time_value;

        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();
    return 0;
}
