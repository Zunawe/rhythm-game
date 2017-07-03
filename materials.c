#include "graphics_utils.h"

void set_material_properties(MaterialProperties props){
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, props.ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, props.diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, props.specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, props.emission);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, props.shininess);
}
