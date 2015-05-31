/* Vector objects for Asteroids
 *
 * Objects created in Geometers's Sketchpad
 *
 * I'm releasing this code under the WTFPL. You can do whatever you like with
 * it, though I'd appreciate credit and thanks if you find it useful or fun.
 * See LICENSE.txt for details.
 *            -Joe McKenzie / Chupi
 */

#ifndef __ASTEROIDS_OBJECTS_H__
#define __ASTEROIDS_OBJECTS_H__

#define PI 3.14159265358979323846

//player ship
// -PI/2 because I modeled the ship in the wrong position
// now it faces the same direction it shoots
const double ship_radius = 35.0;
const double ship_p[] = {
	1.61495, -0.66940*PI - PI/2,	//lower left corner
	1.00000,  0.50000*PI - PI/2,	//top
	1.61495, -0.33060*PI - PI/2,	//lower right corner
	1.21227, -0.30877*PI - PI/2,	//up to right end of bottom
	1.21227, -0.69123*PI - PI/2,	//left end of bottom
};

//flame added to player ship when the thruster is active
const double flame_p[] = {
	1.03578, -0.41609*PI - PI/2,	//right
	1.32782, -0.50000*PI - PI/2,	//tip
	1.03578, -0.58391*PI - PI/2,	//left
};

//asteroids
const double roid_radius[] = {80, 40, 20};
const int roid_nsplit = sizeof(roid_radius)/sizeof(roid_radius[0]);
const double roids_p[][24] = {
	{
		1.06765,  0.05172*PI,
		0.52644,  0.27217*PI,
		0.96699,  0.40478*PI,
		1.10489,  0.65537*PI,
		1.09309,  0.85692*PI,
		0.72809,  0.87608*PI,
		1.01805, -0.94299*PI,
		0.98332, -0.74407*PI,
		0.67835, -0.67480*PI,
		0.97739, -0.34487*PI,
		0.68491, -0.07701*PI,
		1.06765,  0.05172*PI,
	}, {
		0.60636, 0.19849*PI,
		0.94163, 0.45607*PI,
		0.46642, 0.49411*PI,
		0.99493, 0.70096*PI,
		1.09309, 0.85692*PI,
		0.56240, 0.98044*PI,
		1.01805, -0.94299*PI,
		0.98332, -0.74407*PI,
		0.63042, -0.72840*PI,
		1.00889, -0.28863*PI,
		1.03000, 0.02137*PI,
		0.60636, 0.19849*PI,
	}, {
		1.00192, -0.00549*PI,
		1.02800, 0.32248*PI,
		1.01320, 0.69018*PI,
		0.55277, 0.83907*PI,
		1.09309, 0.85692*PI,
		0.99553, 0.97789*PI,
		1.01805, -0.94299*PI,
		0.98332, -0.74407*PI,
		0.75311, -0.64758*PI,
		0.49012, -0.42066*PI,
		1.01398, -0.25767*PI,
		1.00192, -0.00549*PI,
	}
};
const int nroid_models = sizeof(roids_p)/sizeof(roids_p[0]);

//bullet
// (written by hand because it's so simple)
const double bullet_p[] = {
	10.00000,  1.00000*PI,
	10.00000,  0.00000*PI,
};

const double logo_radius = 450;
const int logo_len[] = {5, 10, 4, 7, 9, 9, 3, 7, 10};
const double logo_p[][20] = {
	{		//A
		0.80951, 0.95661*PI,
		0.90894, -0.95888*PI,
		1.00323, 0.96503*PI,
		0.96312, 0.99065*PI,
		0.83807, 0.98910*PI,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
	}, {	//S
		0.74412, 0.97417*PI,
		0.71120, 0.95056*PI,
		0.62021, 0.94324*PI,
		0.58157, 0.96692*PI,
		0.61747, 1.00000*PI,
		0.70264, 1.00000*PI,
		0.74442, -0.97265*PI,
		0.70883, -0.94717*PI,
		0.62848, -0.94034*PI,
		0.57842, -0.96478*PI,
	}, {	//T
		0.53118, -0.92924*PI,
		0.36024, -0.89461*PI,
		0.44850, -0.91591*PI,
		0.44670, 0.92079*PI,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
	}, {	//E
		0.15811, 0.75505*PI,
		0.29786, 0.87959*PI,
		0.27685, -0.99385*PI,
		0.16339, -0.98617*PI,
		0.27685, -0.99385*PI,
		0.30055, -0.87260*PI,
		0.16814, -0.75475*PI,
		0, 0,
		0, 0,
		0, 0,
	}, {	//R
		0.11673, 0.60854*PI,
		0.12344, -0.60242*PI,
		0.14480, -0.29985*PI,
		0.15060, -0.19133*PI,
		0.12824, -0.08012*PI,
		0.07106, -0.01590*PI,
		0.03939, -0.95708*PI,
		0.07106, -0.01590*PI,
		0.17679, 0.21378*PI,
		0, 0,
	}, {	//O
		0.30116, 0.11903*PI,
		0.37153, 0.09569*PI,
		0.41155, 0.04129*PI,
		0.41253, -0.04672*PI,
		0.37369, -0.10146*PI,
		0.29403, -0.13039*PI,
		0.22814, -0.08518*PI,
		0.22637, 0.07556*PI,
		0.30116, 0.11903*PI,
		0, 0,
	}, {	//I
		0.51231, 0.06889*PI,
		0.51388, -0.07318*PI,
		0.51231, 0.06889*PI,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		0, 0,
	}, {	//D
		0.71470, 0.04919*PI,
		0.75774, 0.02238*PI,
		0.75827, -0.02535*PI,
		0.71583, -0.05231*PI,
		0.60409, -0.06210*PI,
		0.60275, 0.05842*PI,
		0.71470, 0.04919*PI,
		0, 0,
		0, 0,
		0, 0,
	}, {	//S
		0.83638, 0.02433*PI,
		0.88033, 0.04117*PI,
		0.97190, 0.03728*PI,
		0.99922, 0.02036*PI,
		0.95815, 0.00118*PI,
		0.87298, 0.00129*PI,
		0.83612, -0.02299*PI,
		0.88385, -0.04101*PI,
		0.96485, -0.03755*PI,
		1.00254, -0.01917*PI,
	}
};
const int logo_letters = sizeof(logo_p)/sizeof(logo_p[0]);

#endif
