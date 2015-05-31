/* implementaion of this vector graphics library for oscilloscope
 *
 * Frames are drawn using moveTo/lineTo, which assemble lists of points (struct vlist). When
 * flip is called, sendFrame renders the vlist to an audio clip, which cb_fill_audio plays
 * back in a loop until a newer frame is sent.
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

#define PI 3.14159265358979323846

//VList: x/y point list - sets of 3 Uint16's:
// 1st is X coord, 0 is left, 64k is right
// 2nd is Y coord, 0 is top, 64k is bottom
// 3rd is weight in steps, 0 is dimmest; note that 44100 steps = 1 second to draw this line!!!
//first point's weight is ignored as it's the start position
//all remaining points' weights are the weight going TO that point
struct vlist {
	Uint16 *pts;
	int n;	//number of triplets in pts
};

//right channel is horizontal, left channel is vertical
struct frame {
	Sint16 *samples;
	int n;	//number of left/right *pairs* of samples
};

//working vlist for moveTo/lineTo
struct vlist work;
Uint16 work_pts[(MAX_POINTS)*3];

//currFrame is drawn repeatedly until nextFrame is non-null
//then nextFrame becomes currFrame (and currFrame is freed) when currFrame finishes drawing
struct frame currFrame, nextFrame;

//"screen" dimensions
double xmin=0, xmax=1000, ymin=0, ymax=1000, targetWeight=100;

//orientation
int flipX=0, flipY=0, swapXY=0;

int g_freq;
double g_refresh;

void cb_fill_audio(void *udata, Uint8 *stream, int len);
void sendFrame(struct vlist *vl);

void gfxInit(int freq, int buffer) {
	SDL_AudioSpec aspec;
	Sint16 *initSamps;

	if(freq <= 0 ) freq=44100;
	g_freq = freq;
	if(buffer <= 0) buffer=1024;

	aspec.freq = freq;
	aspec.format = AUDIO_S16SYS;	//accept "Sint16" samples
	aspec.channels = 2;
	aspec.samples = buffer;
	aspec.callback = cb_fill_audio;
	aspec.userdata = NULL;

	if(SDL_OpenAudio(&aspec, NULL) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(1);
	}

	//initialize frames
	memset(&currFrame, 0, sizeof(struct frame));
	memset(&nextFrame, 0, sizeof(struct frame));
	initSamps = malloc(aspec.samples/4*aspec.channels*sizeof(initSamps[0]));
	memset(initSamps, 0, aspec.samples/4*aspec.channels*sizeof(initSamps[0]));
	currFrame.samples = initSamps;
	currFrame.n = aspec.samples/4*aspec.channels;

	//setup working vlist for moveTo/lineTo
	work.pts = work_pts;
	work.n = 0;

	SDL_PauseAudio(0);	
}

//fill the buffer with loops of currFrame, switching to nextFrame if it's available
void cb_fill_audio(void *udata, Uint8 *stream, int len) {
	//len is BYTES
	static int pos = 0;	//position in currFrame (in bytes)
	int left = len;	//bytes we still need to do
	int done = 0;
	int frameLeft;	//bytes left in currFrame
	int toCopy;	//bytes for this memcpy

	while(left > 0) {
		frameLeft = currFrame.n*4 - pos;	// *4 because frame n values are in sample-pairs
		if(frameLeft < left) toCopy = frameLeft;
		else toCopy = left;

		if(toCopy > 0)
			memcpy(stream+done, ((Uint8*)currFrame.samples)+pos, toCopy);

		left -= toCopy;
		done += toCopy;
		pos += toCopy;
		frameLeft -= toCopy;
		if(frameLeft <= 0) {
			if(frameLeft<0) fprintf(stderr, "frameLeft is %d !?!?!?\n", frameLeft);
			//reached the end of this frame
			pos = 0;
			if(nextFrame.samples != NULL) {
				//new frame available!
				//replace currFrame with nextFrame
				free(currFrame.samples);
				memcpy(&currFrame, &nextFrame, sizeof(struct frame));
				memset(&nextFrame, 0, sizeof(struct frame));
			}
		}
	}
}

//submits vl is the next frame to draw, by rendering it to a frame of samples
//if a nextFrame is already waiting, it is overwritten and freed
//**does NOT free vl or its point list**
void sendFrame(struct vlist *vl) {
	int i, pt, steps, pos=0;
	double X, Y, t, nextX, nextY, dX, dY;
	Sint16 iX, iY;
	Sint16 *buf;
	int bufsiz=0;	//buffer size in L/R pairs of samples

	//find buffer size
	for(pt = 1; pt < vl->n; pt++)
		bufsiz += vl->pts[3*pt+2]+1;

	//allocate buffer
	buf = malloc(bufsiz * 4);	// *4 for 2 16-bit samples

	for(pt = 1; pt < vl->n; pt++) {
		X = vl->pts[3*(pt-1)+0];	//start at previous point
		Y = vl->pts[3*(pt-1)+1];
		nextX = vl->pts[3*pt+0];	//head toward current point
		nextY = vl->pts[3*pt+1];
		steps = vl->pts[3*pt+2]+1;
		//fprintf(stderr, "\t%lf, %lf, %d\n", X, Y, steps);	//DEBUG: output points
		dX = (nextX-X)/(double)steps;
		dY = (nextY-Y)/(double)steps;
		for(i=0; i<steps; i++) {
			if(flipX) {
				iX = (Sint16)(((int)X)-32768);
			} else {
				iX = (Sint16)(((int)(0x0ffff-(Uint16)X))-32768);
			}
			if(flipY) {
				iY = (Sint16)(((int)(0x0ffff-(Uint16)Y))-32768);
			} else {
				iY = (Sint16)(((int)Y)-32768);
			}
			if(swapXY) {
				buf[pos++] = iX;
				buf[pos++] = iY;
			} else {
				buf[pos++] = iY;
				buf[pos++] = iX;
			}
			X += dX;
			Y += dY;
		}
	}

	//DEBUG: sanity check
	if(pos != bufsiz*2)
		fprintf(stderr, "sendFrame: calculated %d samples needed, but used %d\n", bufsiz*2, pos);

	//fprintf(stderr, "Frame is %d samples, %lf Hz refresh\n", bufsiz, ((double)g_freq)/bufsiz);	//DEBUG: frame size and refresh rate
	g_refresh = ((double)g_freq)/bufsiz;

	//DEBUG: write frame to raw audio file
	//FILE *f = fopen("frame.raw", "wb");
	//fwrite(buf, bufsiz*4, 1, f);
	//fclose(f);

	//save this to nextFrame
	if(nextFrame.samples != NULL) {
		//DEBUG: warn of dropped frame
		//fprintf(stderr, "sendFrame: dropped frame of size %d because one of size %d didn't finish drawing in time; replacing with one of size %d\n", nextFrame.n, currFrame.n, bufsiz);
		free(nextFrame.samples);
	}
	nextFrame.n = bufsiz;
	nextFrame.samples = buf;
}

//set screen size for moveTo/lineTo
//xleft/xright are the X coordinate of the left/right edge of the screen
//ytop/ybottom similarly
void setScale(double xleft, double xright, double ytop, double ybottom, double weight) {
	xmin = xleft;
	xmax = xright;
	ymin = ytop;
	ymax = ybottom;
	targetWeight = weight;
}

//move the cursor to a point on the screen
void moveTo(double x, double y) {
	lineTo(x, y, 0);
}

//draw a line to a point on the screen
//color ranges from 0 (invisible) to 1 (bright) or more
//if x or y are outside the screen, they simply get clamped to screen edges
void lineTo(double x, double y, double color) {
	//quit if vector list is full for this frame
	if(work.n >= MAX_POINTS) return;

	//scale to 0..65535
	if(xmin == xmax) x=32768;	//X axis flattened ==> go to middle
	else x = ((x-xmin)/(xmax-xmin))*65535;
	if(ymin == ymax) y=32768;	//Y axis flattened ==> go to middle
	else y = ((y-ymin)/(ymax-ymin))*65535;

	//clamp to screen edges
	if(x < 0) x=0;
	else if(x > 65535) x=65535;
	if(y < 0) y=0;
	else if(y > 65535) y=65535;

	if(work.n > 0) {
		double xDist, yDist, lineLen, newColor;
		//there's a previous point we're drawing a line from
		//calculate steps from color and line length
		xDist = work.pts[(work.n-1)*3+0] - x;
		yDist = work.pts[(work.n-1)*3+1] - y;
		lineLen = sqrt(xDist*xDist + yDist*yDist)/65535;
		if(lineLen < 0.00002) lineLen = 5.0/100.0;	//allow "dwelling" on a point to draw a bright dot
		//65 is a good number of steps for a bright line all the way across the screen
		color = color*lineLen*targetWeight;
		if(color < 1.0) color=1.0;
	} else color=0;	//first point must be a moveTo

	//append to list
	work.pts[work.n*3+0] = (Uint16)x;
	work.pts[work.n*3+1] = (Uint16)y;
	work.pts[work.n*3+2] = (Uint16)color;
	work.n++;
}

void flip(int clear) {
	sendFrame(&work);
	if(clear) work.n = 0;
}

void setMode(int mode) {
	flipX = mode&1;
	flipY = mode&2;
	swapXY = mode&4;
}

double getRefreshRate(void) {
	return g_refresh;
}
