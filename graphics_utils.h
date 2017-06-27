#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "SDL/SDL.h"
#include "GL/gl.h"
#include "GL/glu.h"

typedef struct{
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float emission[4];
	float shininess;
} MaterialProperties;

void set_material_properties(MaterialProperties props);
void throw_error(const char *format, ...);
void reverse_bytes(void* x, const int n);
unsigned int load_texture(const char* file);
