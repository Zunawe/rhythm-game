#include "graphics_utils.h"
#include "vector3.h"

#define PI 3.14159265358979323

#define NUM_PATH_STEPS 64
#define NUM_CURVES 128

#define cos_deg(t) cos((t) * PI / 180)
#define sin_deg(t) sin((t) * PI / 180)

SDL_Surface *screen;
char running;

// Perspective Projection Variables
double ar = 1.0;
double dim = 50.0;
double th = 0.01;
double ph = 0.01;

// Number of milliseconds before the key is considered pressed again
unsigned char keypress_period = 200; // milliseconds

// path_points is a list of 3D vectors that define the course
vector3 path_points[(NUM_PATH_STEPS + 1) * NUM_CURVES];
unsigned int current_camera_step = 0;
vector3 current_camera_pos;
vector3 look_camera_pos;

// Set the projection matrix
void project(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(55, ar, 0.2, 8 * dim);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Reshape the window
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

// This function takes a position and direction, and then progresses those vectors
// until the direction is equal to final_direction. The key idea is that the ending direction
// is what is set, not the ending position.
// pathInfo contains the state of the path
// 		pathInfo[0] - Position Vector
// 		pathInfo[1] - Forward Vector
// final_direction is the vector that the path will point in the direction of
// 		after this function returns
// ds is the distance moved per step
// num_steps is the number of steps to take before the curve is complete
// action is the callback for each context along the curve
void traverse_curve(vector3 *pathInfo, vector3 final_direction, double ds, unsigned int num_steps, void (*action)(vector3, vector3)){
	vector3 forward_vector_1 = v3normalize(pathInfo[1]);
	vector3 forward_vector_2 = v3normalize(final_direction);

	// Find the transition vector between initial and final facing
	vector3 df = v3diff(forward_vector_2, forward_vector_1);
	vector3 current_forward = {0, 0, 0};
	for(unsigned int t = 0; t <= num_steps; ++t){
		// Rotate the forward vector slightly
		current_forward.x = forward_vector_1.x + (df.x * t / num_steps);
		current_forward.y = forward_vector_1.y + (df.y * t / num_steps);
		current_forward.z = forward_vector_1.z + (df.z * t / num_steps);
		current_forward = v3normalize(current_forward);

		// Move forward
		pathInfo[0].x += current_forward.x * ds;
		pathInfo[0].y += current_forward.y * ds;
		pathInfo[0].z += current_forward.z * ds;

		// Act on the new position/forward
		action(pathInfo[0], current_forward);
	}
	
	// Set the forward vector to the current state
	pathInfo[1].x = current_forward.x;
	pathInfo[1].y = current_forward.y;
	pathInfo[1].z = current_forward.z;
}

// Callback for traverse curve that puts a vertex at each position
void create_vertex(vector3 pos, vector3 face){
	glVertex3d(pos.x, pos.y, pos.z);
}

void create_path(vector3 pos, vector3 face){
	vector3 up = {0, 1, 0};
	vector3 perpendicular = v3cross(face, up);
	glVertex3d(pos.x + perpendicular.x, pos.y + perpendicular.y, pos.z + perpendicular.z);
	glVertex3d(pos.x, pos.y, pos.z);
	glVertex3d(pos.x - perpendicular.x, pos.y - perpendicular.y, pos.z - perpendicular.z);
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glLoadIdentity();

	if(0){
		gluLookAt(-(2 * dim) * sin_deg(th) * cos_deg(ph), (2 * dim) * sin_deg(ph), (2 * dim) * cos_deg(th) * cos_deg(ph),
			      0, 0, 0,
			      0, cos_deg(ph), 0);
	}
	else{
		gluLookAt(current_camera_pos.x, current_camera_pos.y + 0.15, current_camera_pos.z,
			      look_camera_pos.x, look_camera_pos.y, look_camera_pos.z,
			      0, 1, 0);
	}
	// Lighting
	if(1){
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
	}

	// Draw Objects
	glColor3d(1, 1, 1);

	glPushMatrix();
	glBegin(GL_POINTS);
	for(unsigned int i = 0; i < (NUM_PATH_STEPS + 1) * NUM_CURVES - 1; ++i){
		vector3 up = {0, 1, 0};
		vector3 perpendicular = v3cross(v3diff(path_points[i], path_points[i + 1]), up);
		glVertex3d(path_points[i].x + perpendicular.x, path_points[i].y + perpendicular.y, path_points[i].z + perpendicular.z);
		glVertex3d(path_points[i].x, path_points[i].y, path_points[i].z);
		glVertex3d(path_points[i].x - perpendicular.x, path_points[i].y - perpendicular.y, path_points[i].z - perpendicular.z);
	}
	glEnd();
	glPopMatrix();

	// Unlit Objects
	glDisable(GL_LIGHTING);

	//draw_axes();

	glFlush();
	SDL_GL_SwapBuffers();
}

// Called every 50 ms
void timer(){
	// Move the camera
	current_camera_pos = path_points[current_camera_step];
	look_camera_pos = path_points[current_camera_step + 5];
	++current_camera_step;
	project();
	display();
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

// Callback for traverse_curve that saves position vectors
void record_position(vector3 pos, vector3 face){
	path_points[current_camera_step++] = pos;
}

void init(){
	// Create the path and store it so we don't have to calculate it every frame
	vector3 pos = {0, 0, 0};
	vector3 forward = {1, 1, 0};
	vector3 pathInfo[2] = {pos, forward};
	for(int i = 0; i < NUM_CURVES; ++i){
		vector3 next_forward = {5 * cos(i * i), 2 * sin(i), 5 * sin(i * i * i * 0.2)};
		traverse_curve(pathInfo, next_forward, 0.2, NUM_PATH_STEPS, record_position);
	}
	current_camera_step = 0;

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
	// Timers
	double t = 0;
	double key_timer = 0;
	double move_timer = 50;

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
					key_timer = t + 500;
					break;
			}
		}

		if(t - key_timer < keypress_period){
			handle_keypress();
			key_timer = t;
		}

		if(t - move_timer > 0){
			timer();
			move_timer = t + 50;
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