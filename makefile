
FILES = src/main.c src/shader.c src/autotile.c src/paths.c src/glad/glad.c
ARGS = -fdiagnostics-color=always -g -Wall -Werror -framework CoreFoundation
PROG_NAME = Tetris

all: build run

build:
	gcc $(ARGS) -Iinclude -lSDL2 $(FILES) -o $(PROG_NAME)

run:
	./$(PROG_NAME)

clean:
	rm -f $(PROG_NAME)
	rm -r Tetris.app
	rm -r Tetris.dSYM

mac: clean build
	mkdir Tetris.app
	mkdir Tetris.app/Contents
	mkdir Tetris.app/Contents/Resources
	mkdir Tetris.app/Contents/MacOS
	cp MacOS/Info.plist Tetris.app/Contents/Info.plist
	cp MacOS/Icon.icns Tetris.app/Contents/Resources/Icon.icns
	cp -R shaders Tetris.app/Contents/Resources/shaders
	cp $(PROG_NAME) Tetris.app/Contents/MacOS/$(PROG_NAME)

