#include "graphics_utils.h"


#define PI 3.14159265358979323

#define NUM_PATH_STEPS 64
#define NUM_CURVES 128

#define cos_deg(t) cos((t) * PI / 180)
#define sin_deg(t) sin((t) * PI / 180)


SDL_Surface *screen;
char running;

char view_mode = 0;

// Perspective Projection Variables
double ar = 1.0;
double dim = 50.0;
double th = 0.01;
double ph = 0.01;

// Number of milliseconds before the key is considered pressed again
unsigned int keypress_period = 5000; // milliseconds

// path_points is a list of 3D vectors that define the course
vector3 path_points[(NUM_PATH_STEPS + 1) * NUM_CURVES][3];
unsigned int current_camera_step = 0;

int ship;


MaterialProperties track_material = {{1.0, 1.0, 1.0, 1.0},
                                     {1.0, 1.0, 1.0, 1.0},
                                     {0.5, 0.5, 0.5, 1.0},
                                     {0.0, 0.0, 0.0, 1.0},
                                     100.0};


vector3 v3x = {1, 0, 0};
vector3 v3y = {0, 1, 0};
vector3 v3z = {0, 0, 1};

// Set the projection matrix
void project(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(55, ar, 0.2, 16 * dim);
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

void circle_vertex(double x, double y, double z){
	glNormal3d(x, y, z);
	glVertex3d(x, y, z);
}

void draw_sphere(){
	int dt = 1;
	int dp = 1;

	glPushMatrix();
	for(unsigned int p = 0; p < 180; p += dp){
		glBegin(GL_QUAD_STRIP);
		for(unsigned int t = 0; t <= 360; t += dt){
			circle_vertex(sin_deg(p) * cos_deg(t), sin_deg(p) * sin_deg(t), cos_deg(p));
			circle_vertex(sin_deg(p + dp) * cos_deg(t), sin_deg(p + dp) * sin_deg(t), cos_deg(p + dp));
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

// NOTE: This could be done between points (rather than between directions) by using Bezier curves/cardinal splines
// This was already implemented by the time the lecture came around
// If there's time, I can update this, but it's good enough until then.
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

// Assumes positive y-axis is up always
void rotate_x_to(vector3 v){
	v = v3normalize(v);
	vector3 xz_component = {v.x, 0, v.z};
	xz_component = v3normalize(xz_component);

	double theta = acos(v3dot(xz_component, v3x)) * 180 / PI;
	theta = v.z < 0 ? theta : -theta;
	double phi = acos(v3dot(v, v3y)) * 180 / PI;
	glRotated(theta, 0, 1, 0);
	glRotated(-phi + 90, 0, 0, 1);
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glLoadIdentity();

	if(view_mode){
		gluLookAt(-dim * sin_deg(th) * cos_deg(ph), dim * sin_deg(ph), dim * cos_deg(th) * cos_deg(ph),
			      0, 0, 0,
			      0, cos_deg(ph), 0);
	}
	else{
		gluLookAt(path_points[current_camera_step][0].x + (0.3 * path_points[current_camera_step][2].x), path_points[current_camera_step][0].y + (0.3 * path_points[current_camera_step][2].y), path_points[current_camera_step][0].z + (0.3 * path_points[current_camera_step][2].z),
			      path_points[current_camera_step + 5][0].x, path_points[current_camera_step + 5][0].y, path_points[current_camera_step + 5][0].z,
			      0, 1, 0);
	}
	// Lighting
	if(1){
		float ambient[]  = {0.2, 0.2, 0.2, 1.0};
		float diffuse[]  = {1.0, 1.0, 1.0, 1.0};
		float specular[] = {0.5, 0.5, 0.5, 1.0};
		float position[] = {path_points[current_camera_step][0].x, path_points[current_camera_step][0].y, path_points[current_camera_step][0].z, 1.0};

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

	set_material_properties(track_material);
	glPushMatrix();
	glBegin(GL_QUADS);
	for(unsigned int i = 0; i < (NUM_PATH_STEPS + 1) * NUM_CURVES - 2; ++i){
		vector3 current_perpendicular = v3scale(v3cross(path_points[i][1], path_points[i][2]), 0.2);
		vector3 next_perpendicular = v3scale(v3cross(path_points[i + 1][1], path_points[i + 1][2]), 0.2);
		
		vector3 normal = path_points[i][2];
		glNormal3d(normal.x, normal.y, normal.z);
		glVertex3d(path_points[i][0].x + current_perpendicular.x, path_points[i][0].y + current_perpendicular.y, path_points[i][0].z + current_perpendicular.z);
		glVertex3d(path_points[i][0].x - current_perpendicular.x, path_points[i][0].y - current_perpendicular.y, path_points[i][0].z - current_perpendicular.z);
		glVertex3d(path_points[i + 1][0].x - next_perpendicular.x, path_points[i + 1][0].y - next_perpendicular.y, path_points[i + 1][0].z - next_perpendicular.z);
		glVertex3d(path_points[i + 1][0].x + next_perpendicular.x, path_points[i + 1][0].y + next_perpendicular.y, path_points[i + 1][0].z + next_perpendicular.z);
	}
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslated(path_points[current_camera_step + 3][0].x + (0.1 * path_points[current_camera_step + 3][2].x),
		         path_points[current_camera_step + 3][0].y + (0.1 * path_points[current_camera_step + 3][2].y),
		         path_points[current_camera_step + 3][0].z + (0.1 * path_points[current_camera_step + 3][2].z));
	glScaled(0.1, 0.1, 0.1);
	rotate_x_to(path_points[current_camera_step + 3][1]);
	glCallList(ship);
	glPopMatrix();

	// Unlit Objects
	glDisable(GL_LIGHTING);
	draw_axes();

	glFlush();
	SDL_GL_SwapBuffers();
	check_error_at("Display");
}

void test_display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glLoadIdentity();

	gluLookAt(-dim * sin_deg(th) * cos_deg(ph), dim * sin_deg(ph), dim * cos_deg(th) * cos_deg(ph),
		      0, 0, 0,
		      0, cos_deg(ph), 0);

	
	if(1){
		float ambient[]  = {0.0, 0.0, 0.0, 1.0};
		float diffuse[]  = {0.8, 0.8, 0.8, 1.0};
		float specular[] = {0.5, 0.5, 0.5, 1.0};
		float position[] = {7.0, 7.0, -7.0, 1.0};

		glEnable(GL_NORMALIZE);
		glEnable(GL_LIGHTING);
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
		glEnable(GL_LIGHT0);

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
		glLightfv(GL_LIGHT0, GL_POSITION, position);
	}

	glPushMatrix();
	glScaled(5, 5, 5);
	glCallList(ship);
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glColor3d(1, 1, 1);
	draw_axes();

	glPushMatrix();
	glTranslated(7, 7, -7);
	draw_sphere();
	glPopMatrix();
	glFlush();
	SDL_GL_SwapBuffers();

	check_error_at("Test Display");
}

// Called every 50 ms
void timer(){
	// Move the camera
	++current_camera_step;
	project();
	display();
	check_error_at("Timer");
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
	if(keys[SDLK_SPACE]){
		view_mode = 1 - view_mode;
	}

	th = fmod(th, 360.0);
	ph = ph > 90 ? 89.9 : ph;
	ph = ph < -90 ? -89.9 : ph;
	project();
}

// Callback for traverse_curve that saves position vectors
void record_position(vector3 pos, vector3 face){
	path_points[current_camera_step][0] = pos;
	path_points[current_camera_step][1] = face;
	path_points[current_camera_step++][2] = v3normalize(v3cross(v3cross(face, v3y), face));
}

void init(){
	// Create the path and store it so we don't have to calculate it every frame
	vector3 pos = {0, 0, 0};
	vector3 forward = {1, 1, 0};
	vector3 pathInfo[2] = {pos, forward};
	for(int i = 0; i < NUM_CURVES; ++i){
		vector3 next_forward = {10 * cos(i * i), 4 * sin(i), 10 * sin(i * i * i * 0.2)};
		traverse_curve(pathInfo, next_forward, 0.2, NUM_PATH_STEPS, record_position);
	}
	current_camera_step = 0;

	running = 1;

	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(1024, 1024, 0, SDL_OPENGL | SDL_RESIZABLE | SDL_DOUBLEBUF);
	if(!screen){
		throw_error("Failed to set video mode\n");
	}
	SDL_WM_SetCaption("Bryce Wilson", "Bryce Wilson");


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	reshape(screen->w, screen->h);

	ship = load_obj("ship.obj");

	check_error_at("Init");
}

void main_loop(void (*display)(void)){
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
					key_timer = t + 5;
					break;
			}
		}

		if(t > key_timer + 5){
			handle_keypress();
			key_timer = t + 5;
		}

		if(t - move_timer > 0){
			timer();
			move_timer = t + 50;
		}

		display();
		check_error_at("Main Loop");
	}
}

void cleanup(){
	SDL_Quit();
}

int main(int argc, char *argv[]){
	init();

	main_loop(display);

	cleanup();
	return 0;
}