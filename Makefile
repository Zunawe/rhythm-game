EXE = project

ifeq "$(OS)" "Windows_NT"
# Windows
SDL_LIBS = $(shell sdl-config --libs)
SDL_CFLAGS = $(shell sdl-config --cflags)
CLEAN = del *.o *.exe
else
ifeq "$(shell uname)" "Darwin"
# OS X
SDL_LIBS = $(shell sdl-config --libs)
SDL_CFLAGS = $(shell sdl-config --cflags)
CLEAN = rm -f *.o $(EXE)
else
# Linux
LIBS = -lGL -lGLU -lm
SDL_LIBS = $(shell sdl-config --libs)
CFLAGS = -O3 -Wall
SDL_CFLAGS = $(shell sdl-config --cflags)
CLEAN = rm -f *.o $(EXE)
endif
endif

$(EXE): $(EXE).o graphics_utils.o vector3.o
	gcc -o $@ $^ $(CFLAGS) $(SDL_CFLAGS) $(LIBS) $(SDL_LIBS)
	
.c.o:
	gcc -c $(CFLAGS) $(SDL_CFLAGS) $<

$(EXE).o: $(EXE).c graphics_utils.h vector3.h
graphics_utils.o: graphics_utils.c graphics_utils.h
vector3.o: vector3.c vector3.h

clean:
	$(CLEAN)

zip:
	zip project.zip $(EXE).c graphics_utils.* vector3.* Makefile README
