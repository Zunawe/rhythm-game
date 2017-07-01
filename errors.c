#include "graphics_utils.h"

void check_error_at(const char *location){
	int error = glGetError();
	if(error){
		fprintf(stderr, "ERROR: %s [%s]\n", gluErrorString(error), location);
	}
}

void throw_error(const char *format, ...){
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

