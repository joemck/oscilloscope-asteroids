/* Simple Asteroids game with vector graphics
 * using my vector graphics library that can display on an oscilloscope or in a window
 *
 * I'm releasing this code under the WTFPL. You can do whatever you like with
 * it, though I'd appreciate credit and thanks if you find it useful or fun.
 * See LICENSE.txt for details.
 *            -Joe McKenzie / Chupi
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "gfx.h"
#include "asteroids_objects.h"

#define PI 3.14159265358979323846

//easily-changed game parameters
#define SAFE_ZONE 150	//radius of safe zone around player where new asteroids won't appear
#define DRAG 0.25	//slowdown rate of player ship
#define THRUST 1.0	//player ship thruster power
#define SPIN (PI/16)	//player ship spin rate
#define MAX_ROIDS 32	//maximum asteroids on screen at once
#define INIT_ROIDS 4	//number of asteroids to generate initially
#define ROID_SPEED 3	//maximum speed for a big asteroid - smaller ones may move faster
#define MAX_BULLETS 5	//maximum bullets on screen
#define BULLET_SPEED 15.0	//bullet flight speed
#define BULLET_RANGE 500	//the screen is 1000 wide and 1000 high
#define RAPIDFIRE_ENABLE 1	//allow rapidfire by holding space?
#define RAPIDFIRE_DELAY 5	//frames between rapidfire shots
#define ROID_RESPAWN_THRESHOLD 5	//make new asteroids when there are fewer than this
#define ROID_RESPAWN_DELAY 40	//min frames between asteroid respawns (currently 20 frames/sec)
#define ROID_RESPAWN_RATE 0.6	//probability an asteroid will respawn after ROID_RESPAWN_DELAY
#define MAX_FRAGMENTS 4	//params for the debris that appears when you die
#define FRAGMENT_MIN_AGE 15
#define FRAGMENT_MAX_AGE 25

struct roid {
	int model;
	int split;
	double angle, spin;
	double posX, posY;
	double spdX, spdY;
};

struct bullet {
	double posX, posY;
	double spdX, spdY;
	double angle;
	int age;
};

struct fragment {
	double posX, posY;
	double spdX, spdY;
	double angle;
	double spin;
	int age;
};

//initialize SDL and the graphics library
void sys_initialize(void) {
	SDL_Surface *screen;

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(320, 240, 0, SDL_ANYFORMAT);
	if (screen == NULL) {
		printf("Unable to set video mode: %s\n", SDL_GetError());
		exit(1);
	}

	gfxInit(44100, 1024);
	setScale(0, 1000, 0, 1000, 100);

	srand(time(NULL));
}

double randReal(double low, double high) {
	return low + rand()/(((double)RAND_MAX + 1) / (high-low));
}

//takes bytes instead of point count for easy use with sizeof
void drawObj(const double *obj, double angle, double radius, double offX, double offY, double bright, int bytes) {
	double r, theta, x, y;
	int i;
	int nPts = bytes/(2*sizeof(obj[0]));

	if(nPts < 2) return;	//need at least 2 points

	//move to first point
	r = obj[0];
	theta = obj[1];
	x = radius * r * cos(angle + theta) + offX;
	y = radius * r * sin(angle + theta) + offY;
	moveTo(x, y);
	lineTo(x, y, bright/2);

	//line to remaining points
	for(i=1; i<nPts; i++) {
		r = obj[2*i+0];
		theta = obj[2*i+1];
		x = radius * r * cos(angle + theta) + offX;
		y = radius * r * sin(angle + theta) + offY;
		lineTo(x, y, bright);
	}
}

//draw a box around the screen, starting at a random location
//helps stabilize the picture on an analog oscilloscope
void recenter(void) {
#ifndef NOBOX
	int i, offs = rand()%4, j;
	static const double box[] = {
		0, 0,
		1000, 0,
		1000, 1000,
		0, 1000,
	};

	moveTo(box[offs*2], box[offs*2+1]);
	for(i=1; i<5; i++) {
		j = 2*((i+offs)%4);
		lineTo(box[j], box[j+1], 0.3);
	}
#endif
}

int main(int argc, char **argv) {
	char title[512];
	SDL_Event ev;
	int running=1;
	int mode = 0;
	int titlescr = 1;

	struct roid roids[MAX_ROIDS];
	char roidValid[MAX_ROIDS];
	int roidRespawn = 0;

	struct bullet bullets[MAX_BULLETS];
	char bulletValid[MAX_BULLETS];

	struct fragment fragments[MAX_FRAGMENTS];
	char fragmentValid[MAX_FRAGMENTS];

	double posX=500, posY=500;
	int shoot=0, spin=0, thrust=0;
	double spdX=0, spdY=0, angle=-PI/2;
	double r, theta, dx, dy, x, y;
	int flame = 0;
	int dead = 0;
	int kills = 0, last_kills = 0;

	int i, j, k;

	sys_initialize();

	printf("\n--------------------------------------------------------------------------------\n");
	printf("--------------------------------------------------------------------------------\n");
	printf("------------------------------ A S T E R O I D S -------------------------------\n");
	printf("--------------------------------------------------------------------------------\n");
	printf("--------------------------------------------------------------------------------\n");
	printf("PROTIP: Picture wrong way round? Press \"M\" on the title screen\n\tto cycle through all possible orientations!\n\n");
	printf("Keys: arrows=thrusters, space=cannon, R=respawn when dead\nPress space to start the game.\n\n");
	printf("The game window must be focussed to receive input.\nPressing keys in the terminal won't work.\n");
	printf("--------------------------------------------------------------------------------\n");

	//zero "valid" arrays
	memset(roidValid, 0, MAX_ROIDS);
	memset(bulletValid, 0, MAX_BULLETS);
	memset(fragmentValid, 0, MAX_FRAGMENTS);

	//make some sample asteroids
	for(i=0; i<7; i++) {
		roids[i].model = rand()%nroid_models;
		roids[i].split = rand()%roid_nsplit;
		roids[i].angle = randReal(-PI, PI);
		roids[i].spin = randReal(-PI/64, PI/64);
		roids[i].spdX = randReal(-ROID_SPEED, ROID_SPEED);
		roids[i].spdY = randReal(-ROID_SPEED, ROID_SPEED);
		roids[i].posX = randReal(0, 1000);;
		roids[i].posY = randReal(300, 1000);;
		roidValid[i] = 1;
	}

	while(running) {
		//handle input
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
				case SDL_KEYDOWN:
					//process pressed keys
					switch(ev.key.keysym.sym) {
						case SDLK_r:
							if(dead) {
								dead=0;
								printf("Respawned\n");
							}
							break;
						case SDLK_SPACE:
							if(titlescr) {
								titlescr=0;
								//reset asteroids
								memset(roidValid, 0, MAX_ROIDS);

								//make some asteroids
								for(i=0; i<INIT_ROIDS; i++) {
									roids[i].model = rand()%nroid_models;
									roids[i].split = 0;
									roids[i].angle = randReal(-PI, PI);
									roids[i].spin = randReal(-PI/64, PI/64);
									roids[i].spdX = randReal(-ROID_SPEED, ROID_SPEED);
									roids[i].spdY = randReal(-ROID_SPEED, ROID_SPEED);
									//reject positions too close to player
									do {
										x = randReal(0, 1000);
										y = randReal(0, 1000);
									} while(x>posX-SAFE_ZONE && x<posX+SAFE_ZONE && y>posY-SAFE_ZONE && y<posY+SAFE_ZONE);
									roids[i].posX = x;
									roids[i].posY = y;
									roidValid[i] = 1;
								}
							} else if(!dead) shoot=RAPIDFIRE_DELAY;
							break;
						case SDLK_UP:
							if(!dead && !titlescr) {
								if(thrust >= 0) thrust=1;
								else thrust=0;	//up+down ==> cancel out
							}
							break;
						case SDLK_DOWN:
							if(!dead && !titlescr) {
								if(thrust <= 0) thrust=-1;
								else thrust=0;	//up+down ==> cancel out
							}
							break;
						case SDLK_LEFT:
							if(!dead && !titlescr) {
								if(spin <= 0) spin=-1;
								else spin=0;	//left+right ==> cancel out
							}
							break;
						case SDLK_RIGHT:
							if(!dead && !titlescr) {
								if(spin >= 0) spin=1;
								else spin=0;	//left+right ==> cancel out
							}
							break;
						case SDLK_m:
							if(titlescr) {
								mode = (mode+1)%8;
								setMode(mode);
							}
							break;
						case SDLK_ESCAPE:
						case SDLK_q:
							running = 0;
							break;
					}
					break;
				case SDL_KEYUP:
					//process released keys
					switch(ev.key.keysym.sym) {
						case SDLK_SPACE:
							shoot=0;
							break;
						case SDLK_UP:
							if(!dead && !titlescr) {
								if(thrust > 0) thrust=0;
								else thrust=-1;	//up+down pressed, then release up
							}
							break;
						case SDLK_DOWN:
							if(!dead && !titlescr) {
								if(thrust < 0) thrust=0;
								else thrust=1;	//up+down pressed, then release down
							}
							break;
						case SDLK_LEFT:
							if(!dead && !titlescr) {
								if(spin < 0) spin=0;
								else spin=1;	//left+right pressed, then release left
							}
							break;
						case SDLK_RIGHT:
							if(!dead && !titlescr) {
								if(spin > 0) spin=0;
								else spin=-1;	//left+right pressed, then release right
							}
							break;
					}
					break;
				case SDL_QUIT:
					running = 0;
					break;
			}
		}

		//update state
		//thrusters
		if(thrust) {
			spdX += thrust * THRUST * cos(angle);
			spdY += thrust * THRUST * sin(angle);
		}
		//spin
		angle += spin * SPIN;
		if(angle > PI) angle -= 2*PI;
		else if(angle < -PI) angle += 2*PI;
		//movement
		posX += spdX;
		posY += spdY;
		//drag
		r = sqrt(spdX*spdX + spdY*spdY);
		theta = atan2(spdY, spdX);
		if(r > DRAG) r -= DRAG;
		else if(r < -DRAG) r += DRAG;
		else r=0.0;
		spdX = r * cos(theta);
		spdY = r * sin(theta);
		//wrap
		if(posX > 1000) posX -= 1000;
		else if(posX < 0) posX += 1000;
		if(posY > 1000) posY -= 1000;
		else if(posY < 0) posY += 1000;

		//shooting
		if(shoot) {
			if(shoot++ >= RAPIDFIRE_DELAY) {
				//if this starts at 0, you have to push space again to shoot again
				//if it's 1, it starts counting up again till the next shot
				shoot = RAPIDFIRE_ENABLE;

				//find an unused bullet
				for(i=0; i<MAX_BULLETS && bulletValid[i]; i++);
				if(i < MAX_BULLETS) {
					//found an unused bullet slot, we can shoot
					bullets[i].posX = posX;
					bullets[i].posY = posY;
					bullets[i].angle = angle;
					bullets[i].spdX = BULLET_SPEED * cos(angle);
					bullets[i].spdY = BULLET_SPEED * sin(angle);
					bullets[i].age = (int)(BULLET_RANGE/BULLET_SPEED);
					bulletValid[i] = 1;
				}
			}
		}

		//draw screen
		//logo
		if(titlescr) {
			for(i=0; i<logo_letters; i++) {
				drawObj(logo_p[i], 0, logo_radius, 500, 150, 1.0, logo_len[i]*2*sizeof(logo_p[0][0]));
				recenter();
			}
		}

		//ship
		if(!dead && !titlescr) {
			drawObj(ship_p, angle, ship_radius, posX, posY, 1.0, sizeof(ship_p));
			if(thrust>0) flame = !flame; else flame=0;
			if(flame) {
				drawObj(flame_p, angle, ship_radius, posX, posY, 1.0, sizeof(flame_p));
				drawObj(flame_p, angle, ship_radius, posX, posY, 1.0, sizeof(flame_p));
			}
		}

		//update and draw bullets
		for(i=0; i<MAX_BULLETS; i++) {
			if(!bulletValid[i]) continue;
			if(bullets[i].age-- <= 0) bulletValid[i] = 0;
			else {
				struct bullet *b = &bullets[i];
				//update
				b->posX += b->spdX;
				b->posY += b->spdY;
				if(b->posX > 1000) b->posX -= 1000;
				else if(b->posX < 0) b->posX += 1000;
				if(b->posY > 1000) b->posY -= 1000;
				else if(b->posY < 0) b->posY += 1000;

				//collision check
				for(j=0; j<MAX_ROIDS && bulletValid[i]; j++) {
					struct roid *a;

					if(!roidValid[j]) continue;

					a = &roids[j];
					dx = a->posX - b->posX;
					dy = a->posY - b->posY;
					r = roid_radius[a->split];
					//optimized to use 1 extra multiply instead of sqrt
					if(dx*dx + dy*dy < r*r) {
						//bullet hit asteroid
						//destroy bullet
						bulletValid[i] = 0;
						if(++a->split >= roid_nsplit) {
							//asteroid is already at smallest size, destroy it
							roidValid[j] = 0;
							if(kills++ == 0)
								printf("FIRST BLOOD - You've destroyed an asteroid!\n");
						} else {
							//split into 2 new asteroids
							//find a free spot
							for(k=0; k<MAX_ROIDS && roidValid[k]; k++);
							if(k<MAX_ROIDS) {
								//new asteroid goes in position k
								roids[k].model = rand()%nroid_models;
								roids[k].split = a->split;
								roids[k].angle = randReal(-PI, PI);
								roids[k].spin = randReal(-PI/64, PI/64);
								roids[k].spdX = -a->spdX + randReal(-ROID_SPEED, ROID_SPEED);
								roids[k].spdY = -a->spdY + randReal(-ROID_SPEED, ROID_SPEED);
								//if(abs(roids[k].spdX) > 2) roids[k].spdX *= 0.5;
								//if(abs(roids[k].spdY) > 2) roids[k].spdY *= 0.5;
								// +6*... give new asteroids a bit of a jolt so they aren't on top of each other
								roids[k].posX = a->posX + 6*roids[k].spdX;
								roids[k].posY = a->posY + 6*roids[k].spdY;
								roidValid[k] = 1;

								//fix up the old one too
								a->model = rand()%nroid_models;
								a->angle = randReal(-PI, PI);
								a->spin = randReal(-PI/64, PI/64);
								a->spdX += randReal(-ROID_SPEED, ROID_SPEED);
								a->spdY += randReal(-ROID_SPEED, ROID_SPEED);
								//if(abs(a->spdX) > 2) a->spdX *= 0.5;
								//if(abs(a->spdY) > 2) a->spdY *= 0.5;
								a->posX += 6*a->spdX;
								a->posY += 6*a->spdY;
							} else printf("WARNING: out of space for new asteroids!\n");
						}
					}
				}

				//draw it
				if(!titlescr)
					drawObj(bullet_p, b->angle, 1.0, b->posX, b->posY, 1.0, sizeof(bullet_p));
			}
		}

		recenter();

		//update and draw fragments
		for(i=0; i<MAX_FRAGMENTS; i++) {
			if(!fragmentValid[i]) continue;
			if(fragments[i].age-- <= 0) fragmentValid[i] = 0;
			else {
				struct fragment *f = &fragments[i];
				//update
				f->posX += f->spdX;
				f->posY += f->spdY;
				if(f->posX > 1000) f->posX -= 1000;
				else if(f->posX < 0) f->posX += 1000;
				if(f->posY > 1000) f->posY -= 1000;
				else if(f->posY < 0) f->posY += 1000;
				f->angle += f->spin;

				//draw it
				drawObj(bullet_p, f->angle, 2.0, f->posX, f->posY, 1.0, sizeof(bullet_p));
			}
		}

		//process and draw asteroids
		j=0;
		for(i=0; i<MAX_ROIDS; i++) {
			struct roid *a;

			//update
			if(!roidValid[i]) continue;
			j++;	//count asteroids
			a = &roids[i];
			a->angle += a->spin;
			if(a->angle > PI) a->angle -= 2*PI;
			else if(a->angle < -PI) a->angle += 2*PI;
			a->posX += a->spdX;
			a->posY += a->spdY;
			if(a->posX > 1000) a->posX -= 1000;
			else if(a->posX < 0) a->posX += 1000;
			if(titlescr) {
				if(a->posY > 1000 || a->posY-roid_radius[a->split] < 250) a->spdY *= -1;
			} else {
				if(a->posY > 1000) a->posY -= 1000;
				else if(a->posY < 0) a->posY += 1000;
			}

			//collision check
			dx = a->posX - posX;
			dy = a->posY - posY;
			r = ship_radius+roid_radius[a->split];
			//optimized to use 1 extra multiply instead of sqrt
			if(!dead && !titlescr && dx*dx + dy*dy < r*r) {
				int this_kills;
				//generate debris
				for(k=0; k<MAX_FRAGMENTS; k++) {
					fragments[k].posX = posX + randReal(-ship_radius, ship_radius);
					fragments[k].posY = posY + randReal(-ship_radius, ship_radius);
					fragments[k].spdX = randReal(-4, 4);
					fragments[k].spdY = randReal(-4, 4);
					fragments[k].angle = randReal(-PI, PI);
					fragments[k].spin = randReal(-PI/16, PI/16);
					fragments[k].age = (int)randReal(FRAGMENT_MIN_AGE, FRAGMENT_MAX_AGE);
					fragmentValid[k] = 1;
				}
				//reset ship params
				dead = 1;
				posX = posY = 500;
				spdX = spdY = 0;
				angle = -PI/2;
				shoot = spin = thrust = 0;
				flame = 0;
				this_kills = kills - last_kills;
				last_kills = kills;
				if(this_kills == 0)
					printf("You are DEAD, and you've accomplished NOTHING!\n");
				else if(this_kills == 1)
					printf("You are DEAD, and you only destroyed one asteroid!\n");
				else if(this_kills < 10)
					printf("You are DEAD, and you only destroyed %d asteroids!\n", this_kills);
				else
					printf("You are DEAD, but you destroyed %d asteroids! Congratulations!\n", this_kills);
				printf("\tPress R to respawn . . .\n");
			}

			//draw it
			drawObj(roids_p[a->model], a->angle, roid_radius[a->split], a->posX, a->posY, 0.8-0.1*a->split, sizeof(roids_p[0]));
			recenter();
		}

		//asteroid respawn
		if(j < ROID_RESPAWN_THRESHOLD && kills>0 && ++roidRespawn > ROID_RESPAWN_DELAY) {
			if(randReal(0.0, 1.0) < ROID_RESPAWN_RATE) {
				//find asteroid position
				for(i=0; i<MAX_ROIDS && roidValid[i]; i++);
				if(i>=MAX_ROIDS) {
					printf("WARNING: Can't respawn asteroid because the array is full!\n");
				} else {
					//make new asteroid
					roids[i].model = rand()%nroid_models;
					roids[i].split = 0;
					roids[i].angle = randReal(-PI, PI);
					roids[i].spin = randReal(-PI/64, PI/64);
					roids[i].spdX = randReal(-ROID_SPEED, ROID_SPEED);
					roids[i].spdY = randReal(-ROID_SPEED, ROID_SPEED);
					//reject positions too close to player
					do {
						x = randReal(0, 1000);
						y = randReal(0, 1000);
					} while(x>posX-2*SAFE_ZONE && x<posX+2*SAFE_ZONE && y>posY-2*SAFE_ZONE && y<posY+2*SAFE_ZONE);
					roids[i].posX = x;
					roids[i].posY = y;
					roidValid[i] = 1;
				}
			}
			roidRespawn = 0;
		}

		recenter();

		flip(1);
		snprintf(title, sizeof(title), "Asteroids [%d Hz]", (int)(getRefreshRate()+0.5));
		SDL_WM_SetCaption(title, title);
		SDL_Delay(50);
	}

	printf("\nProgram terminating. Showing great courage, you have destroyed %d asteroid(s),\nbut %d more remain.\n\n", kills, rand()+9001);

	return 0;
}
