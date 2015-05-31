/* Vector graphics system drawing on oscilloscope screen via sound card!
 *
 * This draws vector graphics on any oscilloscope that is connected to your sound
 * card's output. The left audio channel should be connected to the vertical
 * input, and the right channel to horizontal. The scope should be set to use an
 * external horizontal drive signal (NOT external *sync*), and adjusted so
 * horizontal and vertical inputs offset the beam about the same amount per volt.
 *
 * The beam can't be switched off, so (hopefully) dim lines may appear between
 * separate sets of vectors. The farther the beam jumps, the dimmer the resulting
 * line, but it also makes a longer line.
 *
 * This system has 3 active frames.
 *    Current frame: Currently being drawn on the scope.
 *    Waiting frame: Already been rendered to a PCM wave, will start drawing when
 *          the current frame finishes.
 *    Work frame: Not yet rendered. Any draw operations work on this frame. Call
 *          flip to shift this to the waiting frame slot.
 *
 * If the current frame finishes drawing and there is no waiting frame, the
 * current frame is drawn again. If there is a waiting frame, the current frame
 * is deleted and the waiting one becomes current, leaving the waiting slot empty.
 *
 * The flip function (below) renders the work frame to a PCM wave and makes it
 * the waiting frame. If there's already a waiting frame, the old waiting frame
 * is dropped.
 *
 * I'm releasing this code under the WTFPL. You can do whatever you like with
 * it, though I'd appreciate credit and thanks if you find it useful or fun.
 * See LICENSE.txt for details.
 *            -Joe McKenzie / Chupi
 */

#ifndef __GFX_H__
#define __GFX_H__

#include "SDL/SDL.h"
#include "SDL/SDL_audio.h"

/* Maximum number of moveTo/lineTo calls on screen at once. If this is exceeded,
 * all further calls to moveTo or lineTo will be ignored. The image will likely
 * become insanely flickery long before this. If you change this, you'll need to
 * recompile gfx.c.                                                           */
#define MAX_POINTS 4096

/* gfxInit: initialize stuff and start SDL audio playing
 *   freq: audio sample frequency to use
 *   The requested frequency must be supported by your sound card.
 *   Standard frequencies are:
 *    8000
 *    11025
 *    16000
 *    22050
 *    32000
 *    44100 - standard CD quality, supported by virtually everything
 *    48000
 *    96000
 *    192000
 *   If you ask for 0 or a negative frequency, you'll get 44100. If you ask for
 *   one your sound card doesn't support, you'll get an error on the console and
 *   the program exits.
 *
 *   buffer: size of audio buffer in samples
 *     If zero or negative, a default value of 1024 is used.
 * */
extern void gfxInit(int freq, int buffer);


/* setScale: set screen coordinates for moveTo/lineTo
 *   The first 4 variables are self-explanitory.
 *   Note that if, say, xleft > xright, the X axis is inverted. This allows you
 *   to orient either axis either way.
 *
 *   weight: scale factor for weights for lineTo. This is a number of audio
 *     samples that will be spent on a lineTo call whose weight is 1.0. Too
 *     low or high values will result in inaccuracy due to the sound card's
 *     frequency response range. Too high values also result in flickering.
 *     A reasonable value is 100.                                             */
extern void setScale(double xleft, double xright, double ytop, double ybottom, double weight);


/* moveTo: move as fast as possible to the specified point
 *   Note that the beam can't be turned off, so some line will still be visible.
 *   The farther you move at once, the dimmer the line.
 *   Many moveTo calls to nearby points may be used to draw smooth curves.    */
extern void moveTo(double x, double y);


/* lineTo: draw a line from the last moveTo or lineTo corredinates to the
 *   specified coordinates. Weight is the brightness of the line, and how long
 *   it takes to draw it. Weight scales the global weight set by setScale.
 *   Values between 0 and 1 should generally be used.                         */
extern void lineTo(double x, double y, double weight);


/* flip: switch the current display to what has been drawn using moveTo/lineTo.
 *   Note that partial frames will never be drawn. New frames submitted using
 *   flip wait until whatever's currently on-screen finishes drawing. If there
 *   is no new frame waiting when a frame finishes drawing, the current one is
 *   drawn again.
 *
 *   clear: if non-zero, the queue of points is also cleared
 *   Use flip(0) if you want to add to what's already been drawn after the call.
 *                                                    (i.e. for a paint program)
 *   Use flip(1) if you want to start with a blank slate again.
 *                     (i.e. for a game, where you want to redraw on each frame) */
extern void flip(int clear);

/* setMode: set the picture orientation mode
 *   mode is a bitmap:
 *     & 1 : mirror X axis
 *     & 2 : mirror Y axis
 *     & 4 : swap X and Y axes                                                */
extern void setMode(int mode);

/* returns the refresh rate of the last submitted frame, in Hz                */
extern double getRefreshRate(void);

#endif
