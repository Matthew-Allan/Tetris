
FILES = src/main.c src/shader.c src/autotile.c src/paths.c src/glad/glad.c
ARGS = -fdiagnostics-color=always -g -Wall -Werror -framework CoreFoundation
INCLUDES = -Iinclude -lSDL2
PROG_NAME = Tetris
PROG_VER = 1.0

all: build run

build:
	gcc $(ARGS) $(INCLUDES) $(FILES) -o $(PROG_NAME)

run:
	./$(PROG_NAME)

clean:
	rm -f $(PROG_NAME)
	rm -f -r Tetris.app
	rm -f -r Tetris.dSYM

APP = $(PROG_NAME).app
APP_CONTENTS = $(APP)/Contents
APP_RESOURCES = $(APP_CONTENTS)/Resources
APP_MAC_OS = $(APP_CONTENTS)/MacOS
BUNDLE_ID = com.suityourselfgames.tetris
ICON = Icon.icns
SUBS = -e 's:PROG_NAME:$(PROG_NAME):' -e 's:BUNDLE_ID:$(BUNDLE_ID):' -e 's:PROG_VER:$(PROG_VER):' -e 's:ICON_FILE:$(ICON):'

mac: clean build
	mkdir -p $(APP_RESOURCES)
	mkdir $(APP_MAC_OS)
	cp MacOS/Info.plist $(APP_CONTENTS)/Info.plist
	cp MacOS/Icon.icns $(APP_RESOURCES)/$(ICON)
	cp -R shaders $(APP_RESOURCES)/shaders
	cp $(PROG_NAME) $(APP_MAC_OS)/$(PROG_NAME)
	sed $(SUBS) MacOS/Info.plist > $(APP_CONTENTS)/Info.plist

