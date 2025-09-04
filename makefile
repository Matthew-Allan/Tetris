
FILES = src/main.c src/shader.c src/autotile.c src/glad/glad.c
ARGS = -fdiagnostics-color=always -g -Wall -Werror

all: build run

build:
	gcc $(ARGS) -Iinclude -lSDL2 $(FILES) -o main.app

run:
	./main.app
