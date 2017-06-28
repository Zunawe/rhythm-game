#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
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

typedef struct{
	double x;
	double y;
	double z;
} vec3;

void normalize(vec3 *v);
void set_material_properties(MaterialProperties props);
void check_error_at(const char *location);
void throw_error(const char *format, ...);
void reverse_bytes(void* x, const int n);
unsigned int load_texture(const char* file);
