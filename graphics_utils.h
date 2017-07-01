#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "SDL/SDL.h"
#include "GL/gl.h"
#include "GL/glu.h"


typedef struct{
	double x;
	double y;
	double z;
} vector3;

typedef struct{
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float emission[4];
	float shininess;
} MaterialProperties;


void check_error_at(const char *location);
void throw_error(const char *format, ...);
void set_material_properties(MaterialProperties props);
int load_obj(const char* file);
unsigned int load_texture(const char* file);

vector3 v3cross(vector3 v1, vector3 v2);
double v3dot(vector3 v1, vector3 v2);
vector3 v3diff(vector3 v1, vector3 v2);
double v3length(vector3 v);
vector3 v3normalize(vector3 v);
vector3 v3scale(vector3 v, double s);
vector3 v3sum(vector3 v1, vector3 v2);
