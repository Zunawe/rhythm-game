#include "graphics_utils.h"

void set_material_properties(MaterialProperties props){
	glMaterialfv(GL_FRONT, GL_AMBIENT, props.ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, props.diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, props.specular);
	glMaterialf(GL_FRONT, GL_SHININESS, props.shininess);
}

void throw_error(const char *format, ...){
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

void reverse_bytes(void* x, const int n){
	int k;
	char *ch = (char*)x;
	for(k = 0; k < n / 2; ++k){
		char tmp = ch[k];
		ch[k] = ch[n - 1 - k];
		ch[n - 1 - k] = tmp;
	}
}

unsigned int load_texture(const char* file){
	unsigned int		texture;		// Texture name
	FILE*				f;				// File pointer
	unsigned short		magic;			// Image magic
	unsigned int		dx, dy, size;	// Image dimensions
	unsigned short		nbp, bpp;		// Planes and bits per pixel
	unsigned char*		image;			// Image data
	unsigned int		off;			// Image offset
	unsigned int		k;				// Counter
	int 				max;			// Maximum texture dimensions

	//  Open file
	f = fopen(file,"rb");
	if(!f){
		throw_error("Cannot open file %s\n", file);
	}
	//  Check image magic
	if(fread(&magic, 2, 1, f) != 1){
		throw_error("Cannot read magic from %s\n", file);
	}
	if(magic != 0x4D42 && magic != 0x424D){
		throw_error("%s is not a BMP file\n", file);
	}
	//  Read header
	if(fseek(f, 8, SEEK_CUR) || fread(&off, 4, 1, f) != 1 ||
	   fseek(f, 4, SEEK_CUR) || fread(&dx, 4, 1, f) != 1 || fread(&dy, 4, 1, f) != 1 ||
	   fread(&nbp, 2, 1, f) != 1 || fread(&bpp, 2, 1, f) != 1 || fread(&k, 4, 1, f) != 1){
		throw_error("Cannot read header from %s\n", file);
	}
	// Reverse bytes on big endian hardware (detected by backwards magic)
	if(magic == 0x424D){
		reverse_bytes(&off, 4);
		reverse_bytes(&dx, 4);
		reverse_bytes(&dy, 4);
		reverse_bytes(&nbp, 2);
		reverse_bytes(&bpp, 2);
		reverse_bytes(&k, 4);
	}
	//  Check image parameters
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
	if(dx < 1 || dx > max)	throw_error("%s image width %d out of range 1-%d\n", file, dx, max);
	if(dy < 1 || dy > max)	throw_error("%s image height %d out of range 1-%d\n", file, dy, max);
	if(nbp != 1)			throw_error("%s bit planes is not 1: %d\n", file, nbp);
	if(bpp != 24)			throw_error("%s bits per pixel is not 24: %d\n", file, bpp);
	if(k != 0)				throw_error("%s compressed files not supported\n", file);
#ifndef GL_VERSION_2_0
	// OpenGL 2.0 lifts the restriction that texture size must be a power of two
	for(k = 1; k < dx; k *= 2);
	if(k != dx)				throw_error("%s image width not a power of two: %d\n",file,dx);
	for(k = 1; k < dy; k *= 2);
	if(k != dy)				throw_error("%s image height not a power of two: %d\n",file,dy);
#endif

	//  Allocate image memory
	size = 3 * dx * dy;
	image = (unsigned char*)malloc(size);
	if(!image)				throw_error("Cannot allocate %d bytes of memory for image %s\n",size,file);
	//  Seek to and read image
	if(fseek(f, off, SEEK_SET) || fread(image, size, 1, f) != 1){
		throw_error("Error reading data from image %s\n", file);
	}
	fclose(f);
	//  reverse_bytes colors (BGR -> RGB)
	for(k = 0; k < size; k += 3)
	{
		unsigned char temp = image[k];
		image[k]   = image[k+2];
		image[k+2] = temp;
	}
	//  Generate 2D texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	//  Copy image
	glTexImage2D(GL_TEXTURE_2D, 0, 3, dx, dy, 0, GL_RGB,GL_UNSIGNED_BYTE, image);
	if(glGetError()){
		throw_error("Error in glTexImage2D %s %dx%d\n", file, dx, dy);
	}
	//  Scale linearly when image size doesn't match
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//  Free image memory
	free(image);
	//  Return texture name
	return texture;
}