#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

int buildShader(GLuint *id, const char* vert_path, const char* frag_path);

#endif