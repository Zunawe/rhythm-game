#include "graphics_utils.h"

void set_material_properties(MaterialProperties props){
	glMaterialfv(GL_FRONT, GL_AMBIENT, props.ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, props.diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, props.specular);
	glMaterialf(GL_FRONT, GL_SHININESS, props.shininess);
}
