#include "graphics_utils.h"

#define PI 3.14159265358979323

#define cos_deg(t) cos((t) * PI / 180)
#define sin_deg(t) sin((t) * PI / 180)

SDL_Surface *screen;
char running;

double ar = 1.0;
double dim = 50.0;

double th = 0.01;
double ph = 0.01;

unsigned char keypress_period = 200; // milliseconds

void project(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(55, ar, dim / 16, 16 * dim);
//	glOrtho(-(dim * ar), dim * ar, -dim, dim, -dim, dim);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void reshape(int width, int height){
	ar = height > 0 ? (double)width / (double)height : 1;
	glViewport(0, 0, width, height);
	project();
}

void draw_axes(){
	glBegin(GL_LINES);
	glVertex3d(0, 0, 0);
	glVertex3d(dim / 2, 0, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, dim / 2, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, dim / 2);
	glEnd();
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

vec3 draw_curve(vec3 pos, vec3 forward1, vec3 forward2, double speed, unsigned int num_steps){
	vec3 df = {forward2.x - forward1.x, forward2.y - forward1.y, forward2.z - forward1.z};

	glBegin(GL_LINE_STRIP);
	vec3 current_forward = {forward1.x, forward1.y, forward1.z};
	glVertex3d(pos.x, pos.y, pos.z);
	for(double t = 0.0; t < 1.0; t += 1.0 / num_steps){
		current_forward.x = forward1.x + (df.x * t);
		current_forward.y = forward1.y + (df.y * t);
		current_forward.z = forward1.z + (df.z * t);
		normalize(&current_forward);

		pos.x += current_forward.x * speed;
		pos.y += current_forward.y * speed;
		pos.z += current_forward.z * speed;

		glVertex3d(pos.x, pos.y, pos.z);
	}
	glEnd();
	
	return pos;
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glLoadIdentity();

	gluLookAt(-(2 * dim) * sin_deg(th) * cos_deg(ph), (2 * dim) * sin_deg(ph), (2 * dim) * cos_deg(th) * cos_deg(ph),
		      0, 0, 0,
		      0, cos_deg(ph), 0);

	// Lighting
	float ambient[]  = {0.8, 0.8, 0.8, 1.0};
	float diffuse[]  = {0.7, 0.7, 0.7, 1.0};
	float specular[] = {0.2, 0.2, 0.2, 1.0};
	float position[] = {0.0, 0.0, 0.0, 1.0};

	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	// Draw Objects
	glColor3d(1, 1, 1);

	glPushMatrix();
	vec3 startPos = {1, 1, 1};
	vec3 facing1 = {1, 0, 0};
	vec3 facing2 = {0, 1, 1};
	vec3 facing3 = {-1, -1, -1};
	vec3 newPos = draw_curve(startPos, facing1, facing2, 5, 10);
	draw_curve(newPos, facing2, facing3, 1.25, 5);
	glPopMatrix();

	// Unlit Objects
	glDisable(GL_LIGHTING);

	draw_axes();

	glFlush();
	SDL_GL_SwapBuffers();
}

void handle_keypress(){
	Uint8* keys = SDL_GetKeyState(NULL);
	if(keys[SDLK_ESCAPE]){
		running = 0;
	}
	if(keys[SDLK_UP]){
		ph += 5;
	}
	if(keys[SDLK_DOWN]){
		ph -= 5;
	}
	if(keys[SDLK_LEFT]){
		th += 5;
	}
	if(keys[SDLK_RIGHT]){
		th -= 5;
	}

	th = fmod(th, 360.0);
	ph = ph > 90 ? 89.9 : ph;
	ph = ph < -90 ? -89.9 : ph;
	project();
}

void init(){
	running = 1;

	SDL_Init(SDL_INIT_VIDEO);

	screen = SDL_SetVideoMode(1024, 1024, 0, SDL_OPENGL | SDL_RESIZABLE | SDL_DOUBLEBUF);
	if(!screen){
		throw_error("Failed to set video mode\n");
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	reshape(screen->w, screen->h);
}

void main_loop(){
	double t = 0;
	double tp = 0;
	while(running){
		t = SDL_GetTicks();

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_VIDEORESIZE:
					screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 0, SDL_OPENGL | SDL_RESIZABLE | SDL_DOUBLEBUF);
					reshape(screen->w, screen->h);
					break;
				case SDL_QUIT:
					running = 0;
					break;
				case SDL_KEYDOWN:
					handle_keypress();
					tp = t + 500;
					break;
			}
		}

		if(t - tp < keypress_period){
			handle_keypress();
			tp = t;
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