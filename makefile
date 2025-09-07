# General
FILES = src/main.c src/shader.c src/autotile.c src/paths.c src/glad/glad.c
ARGS = -fdiagnostics-color=always -g -Wall -Werror -framework CoreFoundation
INCLUDES = -Iinclude -lSDL2
PROG_NAME = Tetris
PROG_VER = 1.0

# For MAC
APP = $(PROG_NAME).app
APP_CONTENTS = $(APP)/Contents
APP_RESOURCES = $(APP_CONTENTS)/Resources
APP_FRAMEWORKS = $(APP_CONTENTS)/Frameworks
APP_MAC_OS = $(APP_CONTENTS)/MacOS
BUNDLE_ID = com.suityourselfgames.tetris
ICON = Icon.icns
SUBS = -e 's:PROG_NAME:$(PROG_NAME):' -e 's:BUNDLE_ID:$(BUNDLE_ID):' -e 's:PROG_VER:$(PROG_VER):' -e 's:ICON_FILE:$(ICON):'

# Build and run the executable.
all: build run

# Build the executable
build:
	gcc $(ARGS) $(INCLUDES) $(FILES) -o $(PROG_NAME)

# Run the program
run:
	./$(PROG_NAME)

# Clean any files that may have been created on build
clean:
	rm -f $(PROG_NAME)
	rm -f -r $(APP)
	rm -f -r $(PROG_NAME).dSYM

# Create a .app package
mac: clean build
	mkdir -p $(APP_RESOURCES)
	mkdir $(APP_MAC_OS)
	mkdir $(APP_FRAMEWORKS)
	cp MacOS/libSDL2-2.0.0.dylib $(APP_FRAMEWORKS)/libSDL2-2.0.0.dylib
	cp MacOS/Info.plist $(APP_CONTENTS)/Info.plist
	cp MacOS/Icon.icns $(APP_RESOURCES)/$(ICON)
	cp -R shaders $(APP_RESOURCES)/shaders
	cp $(PROG_NAME) $(APP_MAC_OS)/$(PROG_NAME)
	install_name_tool -change $(shell brew --prefix sdl2)/lib/libSDL2-2.0.0.dylib @executable_path/../Frameworks/libSDL2-2.0.0.dylib Tetris.app/Contents/MacOS/Tetris
	sed $(SUBS) MacOS/Info.plist > $(APP_CONTENTS)/Info.plist
	zip -r Tetris.app.zip Tetris.app

