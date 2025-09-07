#include "shader.h"
#include "paths.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

char *getFileText(const char *path) {
    char *abs_path = get_path(path);
    FILE *file = fopen(abs_path, "r");
    free(abs_path);

    if(!file) {
        printf("Couldn't open file; %s\n", strerror(errno));
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char *contents = (char *) malloc(size + 1);
    if(!contents) {
        printf("Couldn't allocate memory for file buffer.\n");
        return NULL;
    }

    contents[size] = '\0';
    fread(contents, 1, size, file);

    fclose(file);
    return contents;
}

int compileShader(const char *path, GLenum shader_type, GLuint *shader) {
    printf("Compiling %s...\n", path);
    const char *shad_src = getFileText(path);
    if(!shad_src) {
        return -1;
    }

    *shader = glCreateShader(shader_type);
    glShaderSource(*shader, 1, &shad_src, NULL);
    glCompileShader(*shader);

    free((void *) shad_src);

    int success;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);

    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(*shader, 512, NULL, infoLog);
        printf("Shader compilation failed: %s\n", infoLog);
        return -1;
    }

    return 0;
}

int buildShader(GLuint *id, const char* vert_path, const char* frag_path) {
    // Compile the vertex and fragment shaders.
    GLuint vert_shad, frag_shad;
    if(
        compileShader(vert_path, GL_VERTEX_SHADER, &vert_shad) < 0 ||
        compileShader(frag_path, GL_FRAGMENT_SHADER, &frag_shad) < 0
    ) {
        return -1;
    }

    // Create the shader program.
    *id = glCreateProgram();

    // Attach and link the shaders in the program.
    glAttachShader(*id, vert_shad);
    glAttachShader(*id, frag_shad);
    glLinkProgram(*id);

    // Check for errors.
    int success;
    glGetProgramiv(*id, GL_LINK_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetProgramInfoLog(*id, 512, NULL, infoLog);
        printf("Shader linking failed: %s\n", infoLog);
        return -1;
    }

    // Clear up the shaders.
    glDeleteShader(vert_shad);
    glDeleteShader(frag_shad);

    return 0;
}