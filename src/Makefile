#makefile for Asteroids
#
# Note: If you're on MacOS X and want apps instead of Terminal programs,
#   use "make macapps" to package them into stupid-simple .app bundles.
#   WARNING: This means you won't see terminal output, so you won't be
#   informed of "M" to rotate the picture on the title screen or "R" to
#   respawn when you die.
#
# The mac-redist target builds redistributable Mac .apps that can run
# using SDL.framework from libsdl.org, without MacPorts. Normally the
# MacPorts version of SDL is required to run SDL apps created with
# anything other than XCode. Building this target requires:
#   -MacPorts SDL with libSDLmain.a in /opt/local/lib
#       -must be installed as Universal Binary
#        (sudo port install libsdl +universal)
#        or edit the gcc command lines to remove the -arch args, but
#        then you don't get a proper Universal Binary that's suitable
#        for redistribution!
#   -libsdl.org SDL.framework installed normally
#   -the MacPorts SDL must be a reasonably close version to the
#        libsdl.org one
# But running the resulting binaries/.app bundles doesn't require
# **ANY** SDL installation since the .apps include one!
#
# TL;DR: The normal targets require a functioning MacPorts SDL. The
# mac-redist target requires both MacPorts SDL and the official SDL.
#
# 5 Jun 2015: If you add -DNOBOX to CFLAGS here, it won't draw the border
# or the ray randomization it causes.
# 
# I'm releasing this code under the WTFPL. You can do whatever you like with
# it, though I'd appreciate credit and thanks if you find it useful or fun.
# See LICENSE.txt for details.
#            -Joe McKenzie / Chupi

CC=gcc
CFLAGS=$(shell sdl-config --cflags)
LDFLAGS=$(shell sdl-config --libs)

HFILES=asteroids_objects.h gfx.h
EXEC=asteroids-scope asteroids-window

.PHONY: all clean macapps

all: asteroids-scope asteroids-window

asteroids-scope: main.o gfx.o ${HFILES}
	${CC} -o $@ main.o gfx.o ${LDFLAGS}

asteroids-window: main.o gfx_debug.o ${HFILES}
	${CC} -o $@ main.o gfx_debug.o ${LDFLAGS}

macapps: ${EXEC}
	rm -rf asteroids-scope.app asteroids-window.app
	mkdir -p asteroids-scope.app/Contents/MacOS/
	cp asteroids-scope asteroids-scope.app/Contents/MacOS/
	mkdir -p asteroids-window.app/Contents/MacOS/
	cp asteroids-window asteroids-window.app/Contents/MacOS/

mac-redist: clean
	gcc ${CFLAGS} -arch i386 -arch x86_64 -c -o main.o main.c
	gcc ${CFLAGS} -arch i386 -arch x86_64 -c -o gfx.o gfx.c
	gcc ${CFLAGS} -arch i386 -arch x86_64 -c -o gfx_debug.o gfx_debug.c
	gcc -arch i386 -arch x86_64 -Wl,-framework,Cocoa -framework SDL /opt/local/lib/libSDLmain.a -o asteroids-window main.o gfx_debug.o
	gcc -arch i386 -arch x86_64 -Wl,-framework,Cocoa -framework SDL /opt/local/lib/libSDLmain.a -o asteroids-scope main.o gfx.o
	mkdir -p asteroids-scope.app/Contents/MacOS/
	cp asteroids-scope asteroids-scope.app/Contents/MacOS/
	mkdir -p asteroids-scope.app/Contents/Frameworks/
	cp -a /Library/Frameworks/SDL.framework asteroids-scope.app/Contents/Frameworks
	mkdir -p asteroids-window.app/Contents/MacOS/
	cp asteroids-window asteroids-window.app/Contents/MacOS/
	mkdir -p asteroids-window.app/Contents/Frameworks/
	cp -a /Library/Frameworks/SDL.framework asteroids-window.app/Contents/Frameworks

.c.o:
	${CC} -c -o $@ $^ ${CFLAGS}

clean:
	rm -rf *.o ${EXEC} asteroids-scope.app asteroids-window.app
