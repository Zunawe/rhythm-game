#include <math.h>
#include "graphics_utils.h"

#define PI 3.14159265358979323

#define cos_deg(t) cos((t) * PI / 180)
#define sin_deg(t) sin((t) * PI / 180)

SDL_Surface *screen;
char running;

double ar = 1.0;
double dim = 50.0;

double th = -90.0;
double ph = 90.0;

void project(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(55, ar, dim / 4, 4 * dim);
	glOrtho(-ar*dim,ar*dim,-dim,+dim,-dim,+dim);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void reshape(int width, int height){
	ar = height > 0 ? (double)width / (double)height : 1;
	glViewport(0, 0, width, height);
	project();
}

void draw_torus(double R, double r){
	double dt = 1;
	double dT = 1;
	glPushMatrix();
	for(unsigned int T = 0; T < 360; T += dT){
		glBegin(GL_QUAD_STRIP);
		for(unsigned int t = 0; t <= 360; t += dt){
			glTexCoord2f(0.5 + (t / 360.0), T / 360.0);
			glNormal3d(sin_deg(T) * cos_deg(t), sin_deg(t), cos_deg(T) * cos_deg(t));								glVertex3d((R * sin_deg(T)) + (r * cos_deg(t)) * sin_deg(T), r * sin_deg(t), (R * cos_deg(T)) + (r * cos_deg(t)) * cos_deg(T));
			glTexCoord2f(0.5 + ((t + dt) / 360.0), (T + dT) / 360.0);
			glNormal3d(sin_deg(T + dT) * cos_deg(t + dt), sin_deg(t + dt), cos_deg(T + dT) * cos_deg(t + dt));		glVertex3d((R * sin_deg(T + dT)) + (r * cos_deg(t)) * sin_deg(T + dT), r * sin_deg(t), (R * cos_deg(T + dT)) + (r * cos_deg(t)) * cos_deg(T + dT));
		}
		glEnd();
	}
	glPopMatrix();
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glLoadIdentity();

//	gluLookAt(-(2 * dim) * sin_deg(th) * cos_deg(ph), (2 * dim) * sin_deg(ph), (2 * dim) * cos_deg(th) * cos_deg(ph),
//		      0, 0, 0,
//		      0, cos_deg(ph), 0);

	glColor3d(1, 1, 1);

	glPushMatrix();
	//draw_torus(10, 5);
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_QUADS);
		glColor3f(1, 0, 0);		glVertex2d(0, 0);
        glColor3f(1, 1, 0);		glVertex2d(0.5, 0);
        glColor3f(1, 0, 1);		glVertex2d(0.5, 0.5);
        glColor3f(1, 1, 1);		glVertex2d(0, 0.5);
	glEnd();
	glPopMatrix();

	glFlush();
	SDL_GL_SwapBuffers();
}

void init(){
	running = 1;

	SDL_Init(SDL_INIT_VIDEO);

	screen = SDL_SetVideoMode(512, 512, 0, SDL_OPENGL | SDL_RESIZABLE | SDL_DOUBLEBUF);
	if(!screen){
		throw_error("Failed to set video mode\n");
	}

	reshape(screen->w, screen->h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void main_loop(){
	while(running){
		double dt = SDL_GetTicks();

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_VIDEORESIZE:
					break;
				case SDL_QUIT:
					running = 0;
					break;
				case SDL_KEYDOWN:
					break;
			}
		}

		display();
	}
}

void cleanup(){
	SDL_Quit();
}

int main(int argc, char *argv[]){
	init();

	main_loop();

	cleanup();
	return 0;
}