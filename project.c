#include "graphics_utils.h"
#include "SDL_mixer.h"

#define PI 3.14159265358979323

#define NUM_INTERPOLATED_STEPS 64
#define NUM_KNOTS 120
#define NUM_BARRIERS 82
#define NUM_BUTTONS 84
#define NUM_THINGS 10

#define cos_deg(t) cos((t) * PI / 180)
#define sin_deg(t) sin((t) * PI / 180)


typedef struct{
	unsigned int step;
	unsigned char broken;
} barrier;

typedef struct{
	unsigned int step;
	unsigned char pressed;
} button;

typedef struct{
	double R;
	double r;
	vector3 up;
	MaterialProperties material;
	double lifetime;
} pulse;


// General Program
SDL_Surface *screen;
char running;

char car_down = 0;
char hit = 0;
double hit_timer = 0;

// Perspective Projection Variables
double ar = 1.0;

// Locations for drawing objects
vector3 knots[NUM_KNOTS];
vector3 path_points[(NUM_KNOTS - 1) * NUM_INTERPOLATED_STEPS][3];
unsigned int current_camera_step = 0;
unsigned int car_camera_step = 2;
barrier barriers[NUM_BARRIERS] = {{164}, {168}, {172}, {176}, {180}, {184}, {188}, {292}, {296}, {300}, {304}, {308}, {312}, {316}, {356}, {360}, {484}, {488}, {632}, {635}, {638}, {760}, {763}, {766}, {868}, {872}, {928}, {932}, {936}, {940}, {948}, {956}, {992}, {996}, {1000}, {1004}, {1012}, {1020}, {1120}, {1122}, {1124}, {1126}, {1128}, {1136}, {1138}, {1140}, {1142}, {1144}, {1184}, {1186}, {1188}, {1190}, {1192}, {1200}, {1202}, {1204}, {1206}, {1208}, {1210}, {1212}, {1214}, {1248}, {1250}, {1252}, {1254}, {1256}, {1264}, {1266}, {1268}, {1270}, {1272}, {1312}, {1314}, {1316}, {1318}, {1320}, {1328}, {1331}, {1334}, {1336}, {1339}, {1342}};
unsigned int next_barrier = 0;
button buttons[NUM_BUTTONS] = {{0}, {12}, {24}, {64}, {76}, {88}, {128}, {140}, {152}, {192}, {204}, {216}, {256}, {268}, {280}, {320}, {332}, {344}, {352}, {368}, {376}, {384}, {396}, {408}, {448}, {460}, {472}, {480}, {496}, {504}, {512}, {524}, {536}, {560}, {564}, {568}, {576}, {588}, {600}, {624}, {640}, {652}, {664}, {688}, {692}, {696}, {704}, {716}, {728}, {752}, {768}, {780}, {792}, {816}, {820}, {824}, {832}, {844}, {856}, {864}, {880}, {888}, {896}, {908}, {920}, {960}, {972}, {984}, {1024}, {1036}, {1048}, {1088}, {1100}, {1112}, {1152}, {1164}, {1176}, {1216}, {1228}, {1240}, {1280}, {1292}, {1304}, {1344}};
unsigned int next_button = 0;
pulse pulses[5];
unsigned char next_pulse = 0;
vector3 things[] = {{50, -15, 30}, {200, -40, 0}, {300, -10, 20}, {500, 0, -10}, {620, 10, 0}, {700, -20, -40}, {730, 50, 30}, {800, -20, 8}, {900, -30, -25}, {1100, -40, 2}};

// Call lists
int ship_list;
int trail_list;
int barrier_list;
int button_list;
int thing_list;

// Materials
MaterialProperties default_material = {{0.2, 0.2, 0.2, 1.0},
                                     {0.8, 0.8, 0.8, 1.0},
                                     {0.0, 0.0, 0.0, 1.0},
                                     {0.0, 0.0, 0.0, 1.0},
                                     0.0};
MaterialProperties track_material = {{0.1, 0.1, 0.1, 1.0},
                                     {0.2, 0.2, 0.2, 1.0},
                                     {0.6, 0.6, 0.6, 1.0},
                                     {0.0, 0.0, 0.0, 1.0},
                                     32.0};

// Sounds
Mix_Music *music;
Mix_Chunk *boom;
Mix_Chunk *clap;

// Useful vectors
vector3 v3x = {1, 0, 0};
vector3 v3y = {0, 1, 0};
vector3 v3z = {0, 0, 1};
vector3 v30 = {0, 0, 0};


// Set the projection matrix
void project(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(55, ar, 0.5, 200);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Reshape the window
void reshape(int width, int height){
	ar = height > 0 ? (double)width / (double)height : 1;
	glViewport(0, 0, width, height);
	project();
}

void circle_vertex(double x, double y, double z){
	glNormal3d(x, y, z);
	glVertex3d(x, y, z);
}

void draw_torus(double R, double r){
	double dt = 15;
	double dT = 15;
	glPushMatrix();
	for(unsigned int T = 0; T < 360; T += dT){
		glBegin(GL_QUAD_STRIP);
		for(unsigned int t = 0; t <= 360; t += dt){
			glNormal3d(sin_deg(T) * cos_deg(t), sin_deg(t), cos_deg(T) * cos_deg(t));								glVertex3d((R * sin_deg(T)) + (r * cos_deg(t)) * sin_deg(T), r * sin_deg(t), (R * cos_deg(T)) + (r * cos_deg(t)) * cos_deg(T));
			glNormal3d(sin_deg(T + dT) * cos_deg(t + dt), sin_deg(t + dt), cos_deg(T + dT) * cos_deg(t + dt));		glVertex3d((R * sin_deg(T + dT)) + (r * cos_deg(t)) * sin_deg(T + dT), r * sin_deg(t), (R * cos_deg(T + dT)) + (r * cos_deg(t)) * cos_deg(T + dT));
		}
		glEnd();
	}
	glPopMatrix();
}

void draw_sphere(){
	int dt = 15;
	int dp = 15;

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

// Cubic spline algorithm from Wikipedia
// Output is stored in array S
// S is the coefficients for the spline
// S[j](x) = S[0] + S[1](x - S[4]) + S[2](x - S[4])^2 + S[3](x - S[4])^3
void cubic_spline(double x[NUM_KNOTS][2], double S[NUM_KNOTS - 1][5]){
	unsigned int n = NUM_KNOTS - 1;

	double a[n + 1];
	for(unsigned int i = 0; i < n + 1; ++i){
		a[i] = x[i][1];
	}
	double b[n];
	double d[n];
	double h[n];
	for(unsigned int i = 0; i < n; ++i){
		h[i] = x[i + 1][0] - x[i][0];
	}
	double alpha[n];
	for(unsigned int i = 1; i < n; ++i){
		alpha[i] = ((3.0 / h[i]) * (a[i + 1] - a[i])) - ((3.0 / h[i - 1]) * (a[i] - a[i - 1]));
	}
	double c[n + 1];
	double l[n + 1];
	double mu[n + 1];
	double z[n + 1];
	l[0] = 1;
	mu[0] = z[0] = 0.0;

	for(unsigned int i = 1; i < n; ++i){
		l[i] = 2.0 * (x[i + 1][0] - x[i - 1][0]) - (h[i - 1] * mu[i - 1]);
		mu[i] = h[i] / l[i];
		z[i] = (alpha[i] - (h[i - 1] * z[i - 1])) / l[i];
	}
	l[n] = 1;
	z[n] = c[n] = 0;

	for(int j = n - 1; j >= 0; --j){
		c[j] = z[j] - (mu[j] * c[j + 1]);
		b[j] = ((a[j + 1] - a[j]) / h[j]) - ((h[j] * (c[j + 1] + (2.0 * c[j]))) / 3.0);
		d[j] = (c[j + 1] - c[j]) / (3.0 * h[j]);
	}

	for(unsigned int i = 0; i < n; ++i){
		S[i][0] = a[i];
		S[i][1] = b[i];
		S[i][2] = c[i];
		S[i][3] = d[i];
		S[i][4] = x[i][0];
	}
}

// Find a point on the given spline
double interpolate_point(double x, double cs[5]){
	return cs[0] + (cs[1] * (x - cs[4])) + (cs[2] * pow(x - cs[4], 2.0)) + (cs[3] * pow(x - cs[4], 3.0));
}

// Find the derivative at a point on a given spline
double spline_tangent_slope(double x, double cs[5]){
	return cs[1] + (2 * cs[2] * (x - cs[4])) + (3 * cs[3] * pow(x - cs[4], 2.0));
}

// Use knots to draw a smooth curve using cubic_spline
void interpolate_points(vector3 knots[NUM_KNOTS]){
	// We'll just use 1 spline for each dimension
	double x[NUM_KNOTS][2];
	double Sx[NUM_KNOTS - 1][5];
	double y[NUM_KNOTS][2];
	double Sy[NUM_KNOTS - 1][5];
	double z[NUM_KNOTS][2];
	double Sz[NUM_KNOTS - 1][5];

	// Each point is at a non-negative integer t for x(t), y(t), and z(t)
	for(unsigned int i = 0; i < NUM_KNOTS; ++i){
		x[i][0] = i;
		y[i][0] = i;
		z[i][0] = i;

		x[i][1] = knots[i].x;
		y[i][1] = knots[i].y;
		z[i][1] = knots[i].z;
	}

	cubic_spline(x, Sx);
	cubic_spline(y, Sy);
	cubic_spline(z, Sz);

	// Finds NUM_INTERPOLATED_STEPS points between two knots
	// NOTE: if NUM_INTERPOLATED_STEPS isn't divisible by 2,
	// the end of the spline may not be drawn, leading to jaggedness
	double t_step = 1.0 / NUM_INTERPOLATED_STEPS;
	unsigned int next_point = 0;
	for(unsigned int i = 0; i < NUM_KNOTS - 1; ++i){
		for(unsigned int j = 0; j < NUM_INTERPOLATED_STEPS; ++j){
			vector3 pos;
			pos.x = interpolate_point((j * t_step) + i, Sx[i]);
			pos.y = interpolate_point((j * t_step) + i, Sy[i]);
			pos.z = interpolate_point((j * t_step) + i, Sz[i]);
			path_points[next_point][0] = pos;

			vector3 tangent;
			tangent.x = spline_tangent_slope((j * t_step) + i, Sx[i]);
			tangent.y = spline_tangent_slope((j * t_step) + i, Sy[i]);
			tangent.z = spline_tangent_slope((j * t_step) + i, Sz[i]);
			path_points[next_point][1] = v3normalize(tangent);

			path_points[next_point][2] = v3normalize(v3cross(v3cross(tangent, v3y), tangent));

			++next_point;
		}
	}
}

// Initializes a pulse
pulse generate_pulse(vector3 up, double size, double r, double g, double b){
	MaterialProperties material = {{0.0, 0.0, 0.0, 1.0},
	                               {0.0, 0.0, 0.0, 1.0},
	                               {0.0, 0.0, 0.0, 1.0},
	                               {r,   g,   b,   1.0},
	                               0.0};
	pulse new_pulse;
	new_pulse.r = 0.1 * size;
	new_pulse.R = new_pulse.r;
	new_pulse.up = up;
	new_pulse.material = material;
	new_pulse.lifetime = 100;

	return new_pulse;
}

// Deal with hitting obstacles
void check_interaction(){
	// Break Barrier
	if(current_camera_step + car_camera_step == barriers[next_barrier].step && car_down){
	   	pulses[next_pulse] = generate_pulse(path_points[barriers[next_barrier].step][2], 0.1, 1.0, 0.5, 0.0);
		barriers[next_barrier++].broken = 1;
		Mix_PlayChannel(-1, clap, 0);
		next_pulse = (next_pulse + 1) % 5;
	}
	// Press Button
	if(current_camera_step + car_camera_step < buttons[next_button].step + 2 &&
	   current_camera_step + car_camera_step > buttons[next_button].step - 2 &&
	   hit){
	   	pulses[next_pulse] = generate_pulse(path_points[buttons[next_button].step][2], 0.5, 0.0, 0.0, 1.0);
		buttons[next_button++].pressed = 1;
		Mix_PlayChannel(-1, boom, 0);
		next_pulse = (next_pulse + 1) % 5;
	}


	// If passed over something, look at the next one
	if(current_camera_step + 2 > barriers[next_barrier].step){
		++next_barrier;
	}
	if(current_camera_step + 2 > buttons[next_button].step + 2){
		++next_button;
	}
}

// Rotates the x axis to point to the given vector
// First rotates around the y-axis, and then rotates up or down
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

	gluLookAt(path_points[current_camera_step][0].x + (0.5 * path_points[current_camera_step][2].x), path_points[current_camera_step][0].y + (0.5 * path_points[current_camera_step][2].y), path_points[current_camera_step][0].z + (0.5 * path_points[current_camera_step][2].z),
	          path_points[current_camera_step + 8][0].x, path_points[current_camera_step + 8][0].y, path_points[current_camera_step + 8][0].z,
	          0, 1, 0);

	// Lighting
	float ambient[]  = {0.1, 0.1, 0.1, 1.0};
	float diffuse[]  = {0.5, 0.5, 0.5, 1.0};
	float specular[] = {0.5, 0.5, 0.5, 1.0};
	float position[] = {path_points[current_camera_step + car_camera_step][0].x + (0.8 * path_points[current_camera_step + car_camera_step][2].x),
		                path_points[current_camera_step + car_camera_step][0].y + (0.8 * path_points[current_camera_step + car_camera_step][2].y),
		                0.5 * (path_points[current_camera_step + car_camera_step][0].z + (0.8 * path_points[current_camera_step + car_camera_step][2].z)), 1.0};

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

	// Path
	set_material_properties(track_material);
	glPushMatrix();
	glBegin(GL_QUADS);
	for(unsigned int i = 0; i < (NUM_KNOTS - 1) * NUM_INTERPOLATED_STEPS - 1; ++i){
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

	// Buttons
	for(unsigned int i = 0; i < NUM_BUTTONS; ++i){
		if(buttons[i].pressed){
			continue;
		}
		glPushMatrix();
		glTranslated(path_points[buttons[i].step][0].x, path_points[buttons[i].step][0].y, path_points[buttons[i].step][0].z);
		rotate_x_to(path_points[buttons[i].step][1]);
		glRotated(90, 0, 1, 0);
		glScaled(0.05, 0.03, 0.05);
		glCallList(button_list);
		glPopMatrix();
	}

	// Barriers
	for(unsigned int i = 0; i < NUM_BARRIERS; ++i){
		if(barriers[i].broken){
			continue;
		}
		glPushMatrix();
		glTranslated(path_points[barriers[i].step][0].x, path_points[barriers[i].step][0].y, path_points[barriers[i].step][0].z);
		rotate_x_to(path_points[barriers[i].step][1]);
		glRotated(90, 0, 1, 0);
		glScaled(0.2, 0.1, 0.1);
		glCallList(barrier_list);
		glPopMatrix();
	}

	// Things
	for(unsigned int i = 0; i < NUM_THINGS; ++i){
		glPushMatrix();
		glTranslated(things[i].x, things[i].y, things[i].z);
		glScaled(10, 10, 10);
		glCallList(thing_list);
		glPopMatrix();
	}

	// Ship
	glPushMatrix();
	vector3 car_pos = path_points[current_camera_step + car_camera_step][0];
	if(!car_down){
		car_pos = v3sum(car_pos, v3scale(path_points[current_camera_step + car_camera_step][2], 0.01));
	}
	glTranslated(car_pos.x, car_pos.y, car_pos.z);
	glScaled(0.2, 0.2, 0.2);
	rotate_x_to(path_points[current_camera_step + car_camera_step][1]);
	glCallList(ship_list);
	glPopMatrix();


	// Tranlucent Objects
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(0);

	// Pulses
	for(unsigned int i = 0; i < 5; ++i){
		pulse p = pulses[i];
		if(p.lifetime <= 0.0){
			continue;
		}
		set_material_properties(p.material);
		glPushMatrix();
		glTranslated(car_pos.x, car_pos.y, car_pos.z);
		rotate_x_to(path_points[current_camera_step + 5][2]);
		glRotated(90, 0, 0, 1);
		draw_torus(p.R, p.r);
		glPopMatrix();
	}

	// Trail
	glPushMatrix();
	vector3 trail_pos = v3sum(v3sum(car_pos, v3scale(path_points[current_camera_step + car_camera_step][1], -0.35)), v3scale(path_points[current_camera_step + car_camera_step][2], 0.076));
	glTranslated(trail_pos.x, trail_pos.y, trail_pos.z);
	rotate_x_to(v3diff(trail_pos, car_pos));
	glScaled(0.1 * sin_deg(current_camera_step * 80) + 0.2, 0.08, 0.08);
	glCallList(trail_list);
	glPopMatrix();

	float tail_ambient[] = {0.0, 0.0, 0.0, 1.0};
	float tail_diffuse[] = {0.0, 0.0, 0.1, 1.0};
	float tail_specular[] = {0.0, 0.0, 0.8, 1.0};
	float tail_position[] = {trail_pos.x, trail_pos.y, trail_pos.z, 1.0};

	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT1, GL_AMBIENT, tail_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, tail_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, tail_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, tail_position);

	glDisable(GL_BLEND);
	glDepthMask(1);


	// Unlit Objects
	glDisable(GL_LIGHTING);

	glPushMatrix();
	glTranslated(path_points[current_camera_step + car_camera_step][0].x + (0.8 * path_points[current_camera_step + car_camera_step][2].x),
			     path_points[current_camera_step + car_camera_step][0].y + (0.8 * path_points[current_camera_step + car_camera_step][2].y),
			     0.5 * (path_points[current_camera_step + car_camera_step][0].z + (0.8 * path_points[current_camera_step + car_camera_step][2].z)));
	glScaled(0.01, 0.01, 0.01);
	draw_sphere();
	glPopMatrix();

	glFlush();
	SDL_GL_SwapBuffers();
	check_error_at("Display");
}

// Called every 48.875 ms
// Incrementing the car's position every 48.875 ms makes the math easy
// for finding positions at 160 BPM (the speed of this song)
void timer(){
	// Move the camera
	++current_camera_step;
	if(current_camera_step >= (NUM_KNOTS - 1) * NUM_INTERPOLATED_STEPS - 20){
		running = 0;
	}

	// Grow pulses and dim their color
	for(unsigned int i = 0; i < 5; ++i){
		pulses[i].lifetime -= 20;
		pulses[i].R += 0.1;
		pulses[i].material.emission[0] *= pulses[i].lifetime / 100.0 / 2.0;
		pulses[i].material.emission[1] *= pulses[i].lifetime / 100.0 / 2.0;
		pulses[i].material.emission[2] *= pulses[i].lifetime / 100.0 / 2.0;
	}
	project();
	display();
	check_error_at("Timer");
}

void handle_keypress(){
	hit = 0;

	Uint8* keys = SDL_GetKeyState(NULL);
	if(keys[SDLK_ESCAPE]){
		running = 0;
	}
	// hit will make sure buttons are only pressed if you press space on them
	// and not when you hold space through them.
	// hit_timer prevents the player from spamming space to hit everything
	if(keys[SDLK_SPACE]){
		if(hit_timer <= 0){
			if(!car_down){
				hit = 1;
				hit_timer = 150;
			}
			car_down = 1;
		}
		else if(car_down){
			car_down = 1;
		}
	}
	else{
		car_down = 0;
	}

	project();
}

void init(){
	// Create the path and store it so we don't have to calculate it every frame
	for(unsigned int i = 0; i < NUM_KNOTS; ++i){
		vector3 next = {i * 50, 2 * cos(i), 5 * sin(i)};
		knots[i] = next;
	}
	interpolate_points(knots);
	current_camera_step = 0;

	// There are two beats of rest before hitting the first sound
	for(unsigned int i = 0; i < NUM_BUTTONS; ++i){
		buttons[i].step += 16;
	}
	for(unsigned int i = 0; i < NUM_BARRIERS; ++i){
		barriers[i].step += 16;
	}

	// Set program to running
	running = 1;

	// Initialize window with SDL
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(1800, 1024, 0, SDL_OPENGL | SDL_RESIZABLE | SDL_DOUBLEBUF);
	if(!screen){
		throw_error("Failed to set video mode\n");
	}
	SDL_WM_SetCaption("Bryce Wilson", "Bryce Wilson");

	// Initialize music and sound effects
	Mix_Init(MIX_INIT_MP3);
	if(Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096)){
		throw_error("Cannot initialize audio\n");
	}
	music = Mix_LoadMUS("brothers_in_arms.mp3");
	if(!music){
		throw_error("Cannot load music\n");
	}
	clap = Mix_LoadWAV("clap.wav");
	if(!clap){
		throw_error("Cannot load clap\n");
	}
	boom = Mix_LoadWAV("boom.wav");
	if(!boom){
		throw_error("Cannot load boom\n");
	}

	// Load identities and draw a blank screen
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.01, 0.01, 0.02, 1.0);
	reshape(screen->w, screen->h);

	// Load objects
	ship_list = load_obj("ship.obj");
	trail_list = load_obj("trail.obj");
	barrier_list = load_obj("barrier.obj");
	button_list = load_obj("button.obj");
	thing_list = load_obj("thing.obj");
	
	check_error_at("Init");
}

void main_loop(void (*display)(void)){
	// Timers
	double t = 0;			// Global timer
	double dt = 0;			// Change in time since last iteration
	double key_timer = 0;	// Time since key press last handled

	if(Mix_PlayMusic(music, 1)){
		throw_error("Cannot play music\n");
	}
	// Accounts for the delay in the music file's start
	double move_timer = 960;

	while(running){
		dt = t;
		t = SDL_GetTicks();
		dt = t - dt;

		hit_timer -= dt;

		check_interaction();

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

		// Handles key events every 5 milliseconds minimum (usually longer)
		if(t > key_timer + 5){
			handle_keypress();
			key_timer = t + 5;
		}

		// Moves the car forward at a rate that makes math easy at 160 bpm
		if(t > move_timer){
			timer();
			move_timer += 46.875;
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