EXE = project

ifeq "$(OS)" "Windows_NT"
# Windows
SDL_LIBS = $(shell sdl-config --libs) -lSDL_mixer
SDL_CFLAGS = $(shell sdl-config --cflags)
CLEAN = del *.o *.exe
else
ifeq "$(shell uname)" "Darwin"
# OS X
SDL_LIBS = $(shell sdl-config --libs) -lSDL_mixer
SDL_CFLAGS = $(shell sdl-config --cflags)
CLEAN = rm -f *.o $(EXE)
else
# Linux
LIBS = -lGL -lGLU -lm
SDL_LIBS = $(shell sdl-config --libs) -lSDL_mixer
CFLAGS = -O3 -Wall
SDL_CFLAGS = $(shell sdl-config --cflags)
CLEAN = rm -f *.o $(EXE)
endif
endif

$(EXE): $(EXE).o graphics_utils.a
	gcc -o $@ $^ $(CFLAGS) $(SDL_CFLAGS) $(LIBS) $(SDL_LIBS)
	
.c.o:
	gcc -c $(CFLAGS) $(SDL_CFLAGS) $<

graphics_utils.a: errors.o materials.o objects.o textures.o vector3.o
	ar -rcs $@ $^


$(EXE).o: $(EXE).c graphics_utils.a
errors.o: errors.c graphics_utils.a
materials.o: materials.c graphics_utils.a
objects.o: objects.c graphics_utils.a
textures.o: textures.c graphics_utils.a
vector3.o: vector3.c graphics_utils.a


clean:
	$(CLEAN)

zip:
	zip project.zip *.c *.h *.obj *.mtl *.wav *.mp3 Makefile README
