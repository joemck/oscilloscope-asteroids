/* Screen implementation of the oscilloscope vector graphics system.
 *
 * Binary-compatible with it, and draws in a window instead for easier debugging
 * of programs using it. Draws everything with N-pixel-thick lines, supports
 * weights and setScale. Lines drawn over each other add weights, up to white.
 * MoveTo draws a dim line, like on a real scope.
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
#include <math.h>
#include "gfx.h"

#define SIZE 480	//window size (it's always square)
#define LINEWIDTH 1	//controls line thickness (only odd numbers work right)

SDL_Surface *screen;
double xmin, xmax, ymin, ymax, cursX=0, cursY=0;
int flipX=0, flipY=0, swapXY=0;

void gfxInit(int freq, int buffer) {
	static const char title[] = "Vector Output Window";
    screen = SDL_SetVideoMode(SIZE, SIZE, 16, SDL_SWSURFACE);
    if ( screen == NULL ) {
        fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
    }
	 SDL_WM_SetCaption(title, title);
}

static void plot(int x, int y, Uint8 bright) {
	Uint8 r, g, b;
	Uint32 color;
	Uint16 *bufp;
	int bright2;

	//clamp to screen size for attempts to draw out of screen
	if(x<0||x>=SIZE||y<0||y>=SIZE) return;

	if(SDL_MUSTLOCK(screen)) {
		if(SDL_LockSurface(screen) < 0) {
			return;
		}
	}

	//add if we go over a pixel we've already drawn
	bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
	SDL_GetRGB(*bufp, screen->format, &r, &g, &b);
	bright2 = bright + r;
	if(bright2 > 255) bright2=255;
	bright = (Uint8)bright2;
	color = SDL_MapRGB(screen->format, bright, bright, bright);
	*bufp = color;

	if(SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}
}

void setScale(double xleft, double xright, double ytop, double ybottom, double weight) {
	xmin = xleft;
	xmax = xright;
	ymin = ytop;
	ymax = ybottom;
}

void moveTo(double x, double y) {
	lineTo(x, y, 0);
}

static int iabs(int x) {
	return x<0?-x:x;
}

static int clamp(int n) {
	if(n<0) n=0;
	else if(n>=SIZE) n=SIZE-1;
	return n;
}

//standard Bresenham's line algorithm
//adapted from http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
static void lineTo2(int x0, int y0, int x1, int y1, Uint8 shade) {
	int dx, dy, sx, sy, err, e2;

	dx = abs(x1-x0);
	dy = abs(y1-y0);
	if(x0 < x1) sx = 1; else sx = -1;
	if(y0 < y1) sy = 1; else sy = -1;
	err = dx - dy;

	while(1) {
		plot(x0, y0, shade);
		if(x0 == x1 && y0 == y1) break;
		e2 = 2*err;
		if(e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if(e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
}

void lineTo(double x, double y, double weight) {
	int x0, y0, x1, y1, i;
	//disallow completely black lines
	Uint8 wt = (Uint8)(weight*245+10);

	x0 = clamp((int)((SIZE-4)*((cursX - xmin) / (xmax - xmin)))+2);
	y0 = clamp((int)((SIZE-4)*((cursY - ymin) / (ymax - ymin)))+2);
	x1 = clamp((int)((SIZE-4)*((x - xmin) / (xmax - xmin)))+2);
	y1 = clamp((int)((SIZE-4)*((y - ymin) / (ymax - ymin)))+2);

	if(flipX) {
		x0 = SIZE-x0;
		x1 = SIZE-x1;
	}
	if(flipY) {
		y0 = SIZE-y0;
		y1 = SIZE-y1;
	}
	if(swapXY) {
		i = x0;
		x0 = y0;
		y0 = i;
		i = x1;
		x1 = y1;
		y1 = i;
	}

	//move cursor to end of the new line
	cursX = x;
	cursY = y;

	//draw thick lines
	if(iabs(x1-x0) > iabs(y1-y0)) {
		//shallow
		lineTo2(x0, y0, x1, y1, wt);
		for(i=1; i<=LINEWIDTH/2; i++) {
			lineTo2(x0, y0-i, x1, y1-i, wt);
			lineTo2(x0, y0+i, x1, y1+i, wt);
		}
	} else {
		//steep
		lineTo2(x0, y0, x1, y1, wt);
		for(i=1; i<=LINEWIDTH/2; i++) {
			lineTo2(x0-i, y0, x1-i, y1, wt);
			lineTo2(x0+i, y0, x1+i, y1, wt);
		}
	}
}

void flip(int clear) {
	//refresh entire screen from pixel buffer
	SDL_UpdateRect(screen, 0, 0, 0, 0);

	//clear buffer if requested
	if(clear) SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
}

void setMode(int mode) {
	flipX = mode&1;
	flipY = mode&2;
	swapXY = mode&4;
}

double getRefreshRate(void) {
	return 0.0;
}
