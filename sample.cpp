#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>


#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif


#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif


#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glut.h"
#include "keytime.cpp"
#include"Camera.h"
//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// title of these windows:

const char *WINDOWTITLE = "OpenGL / GLUT Sample -- Joe Graphics";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
const GLfloat FOGSTART    = 1.5f;
const GLfloat FOGEND      = 4.f;

// for lighting:

const float	WHITE[ ] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 10000;		// 10000 milliseconds = 10 seconds


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
#define DEMO_Z_FIGHTING
#define DEMO_DEPTH_BUFFER



// Create a Camera object
Camera camera(0.0f, 10.0f, 30.0f);

// Movement keys states
bool moveForward = false, moveBackward = false, strafeLeft = false, strafeRight = false,  moveUp = false,  moveDown = false;

const int BUILDING_TEXTURES = 6;
const int NUM_BUILDINGS = 1;
// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
GLuint	BoxList;				// object display list
GLuint	GroundList;				//gound display list
GLuint	RoadList;				//road display list
GLuint	BuildingsList;			//buildings display list
GLuint	LightsList;				//lights list
GLuint	VegetationList;
GLuint	GrassTexture;			//grass texture object
GLuint	RoadTexture;
GLuint	LightPostTexture;
GLuint	SignList;
GLuint	BuildingTexture[BUILDING_TEXTURES];
GLuint	StopSignTexture;
GLuint	SideWalkTexture;
GLuint	TreeTexture;
GLuint	CarTexture[4];
GLuint	BridgeTexture;
GLuint	BridgeList;
GLuint	TrainList;
GLuint	TrainTexture;
const int NUM_CARS = 7;


	Keytimes TrainXPos;


struct Car
{
	Keytimes xPos;
	Keytimes zPos;
	GLuint displayList;
};

Car		car[NUM_CARS];
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;		// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );
void	DrawPlane(int x, int z);
void	DrawPlane(float x, float y ,float z, float length, float width, float textureRepeatX, float textureRepeatZ);
void	AddTexture(char* file, GLuint& textureObj);
void	InitTextures();
void	InitCarAnimations();
void	AnimateCars();
void	InitGround();
void	InitCars();
void	InitRoad();
void	InitSigns();
void	DrawSign(float x, float z);
void	InitLights();
void	DrawBuildings(float x, float y, float width, float height);
void	InitBuildings();
void	DrawLampPost(float x, float y, int num);
void	InitVegetation();
void	DrawVegetation(float x, float z);
void	handleKeyUp(unsigned char key, int x, int y);
void	mouseMotion(int x, int y);
void	update(int value);
void	InitBridge();
void	DrawBridge(float x, float z);
void	InitTrain();
void	DrawTrain(float x, float z);
void			Axes( float );
void			HsvRgb( float[3], float [3] );
void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);
float			Unit(float [3]);


// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}

float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}

float
Ranf( float low, float high )
{
        float r = (float) rand();               // 0 - RAND_MAX
        float t = r  /  (float) RAND_MAX;       // 0. - 1.

        return   low  +  t * ( high - low );
}

// call this if you want to force your program to use
// a different random number sequence every time you run it:
void
TimeOfDaySeed( )
{
	struct tm y2k;
	y2k.tm_hour = 0;    y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 2000; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  now;
	time( &now );
	double seconds = difftime( now, mktime(&y2k) );
	unsigned int seed = (unsigned int)( 1000.*seconds );    // milliseconds
	srand( seed );
}

// these are here for when you need them -- just uncomment the ones you need:

#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
#include "osucone.cpp"
#include "osutorus.cpp"
#include "bmptotexture.cpp"
#include "loadobjfile.cpp"

//#include "glslprogram.cpp"
#include "vertexbufferobject.cpp"

// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display lists that **will not change**:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_SMOOTH );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( NowProjection == ORTHO )
		glOrtho( -2.f, 2.f,     -2.f, 2.f,     0.1f, 1000.f );
	else
		gluPerspective( 70.f, 1.f,	0.1f, 1000.f );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );


	//update the view matrix
	camera.updateViewMatrix(); 

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[NowColor][0] );
		glPushMatrix();
		glTranslatef(0.0f, 1.f, 0.0f);
		glCallList( AxesList );
		glPopMatrix();
	}
	glColor3f(1.0f, 1.0f, 1.0f);
	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable( GL_NORMALIZE );
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// Enable lighting
	glEnable(GL_LIGHTING); 
	glEnable(GL_LIGHT0);

	// draw the box object by calling up its display list:
	AnimateCars();
	glCallList(GroundList);
	glCallList(LightsList);
	glCallList(BridgeList);
	glCallList(VegetationList);
	glCallList(RoadList);
	glCallList(BuildingsList);
	glCallList(SignList);
	glDisable(GL_TEXTURE_2D);
	
	
#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	NowProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	glutTimerFunc( -1, NULL, 0 );

	glutMotionFunc(mouseMotion);
	glutKeyboardUpFunc(handleKeyUp);
	glutTimerFunc(25, update, 0);  // Update every 25ms (~40 FPS)

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:
	//AddTexture("earth.bmp", GrassTexture);
	InitTextures();
	InitCarAnimations();
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	glutSetWindow( MainWindow );
	InitGround();
	InitRoad();
	InitBuildings();
	InitLights();
	InitVegetation();
	InitSigns();
	InitCars();
	InitTrain();
	InitBridge();

#ifdef NOTDEF
		glColor3f(1., 1., 1.);
		glBegin(GL_TRIANGLES);
		glVertex3f(-dx, -dy, dz);
		glVertex3f(0., -dy, dz + 0.5f);
		glVertex3f(dx, -dy, dz);
		glEnd();
#endif

	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


void DrawPlane(int x, int z)
{
#define XSIDE	x			// length of the x side of the grid
#define X0      (-XSIDE/2.)		// where one side starts
#define NX	100		// how many points in x
#define DX	( XSIDE/(float)NX )	// change in x between the points

#define YGRID	0.f			// y-height of the grid

#define ZSIDE	z			// length of the z side of the grid
#define Z0      (-ZSIDE/2.)		// where one side starts
#define NZ	100			// how many points in z
#define DZ	( ZSIDE/(float)NZ )	// change in z between the points

	glPushMatrix();
	SetMaterial(0.6f, 0.6f, 0.6f, 30.f);
	glNormal3f(0., 1., 0.);
	glTranslatef(0.0f, -.15f, 0.0f);
	for (int i = 0; i < NZ; i++)
	{
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j < NX; j++)
		{
			// Specify texture coordinates (ranging from 0 to 1)
			glTexCoord2f((float)j / (NX - 1), (float)i / (NZ - 1));  // Adjust to match the grid size

			glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 0));

			glTexCoord2f((float)j / (NX - 1), (float)(i + 1) / (NZ - 1));  // Adjust for next row

			glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 1));
		
		}
		glEnd();
	}
	glPopMatrix();
}
void DrawPlane(float x, float y, float z, float length, float width, float textureRepeatX, float textureRepeatZ)
{
	// Grid resolution: Higher values produce smoother rectangles
	const int ax = 100; // Points along the length
	const int az = 100; // Points along the width

	// Step sizes for each grid cell
	const float cx = length / (float)(ax - 1);
	const float dx = width / (float)(az - 1);

	// Bottom-left corner of the rectangle
	const float x1 = x - (length / 2.0f); // Centered on x
	const float z1 = z - (width / 2.0f);  // Centered on z

	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	SetMaterial(0.6f, 0.6f, 0.6f, 30.f);
	glNormal3f(0.0f, 1.0f, 0.0f);       
	glTranslatef(0, y, 0);

	// Draw the rectangle using a grid of quads
	for (int i = 0; i < az - 1; i++)
	{
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j < ax; j++)
		{
			// Compute vertex positions
			float xPos = x1 + cx * j;
			float zPos1 = z1 + dx * i;
			float zPos2 = z1 + dx * (i + 1);

			// Specify texture coordinates (mapped from 0 to 1)
			float texCoordX1 = (float)j / (ax - 1) * textureRepeatX;
			float texCoordZ1 = (float)i / (az - 1) * textureRepeatZ;
			float texCoordX2 = (float)j / (ax - 1) * textureRepeatX;
			float texCoordZ2 = (float)(i + 1) / (az - 1) * textureRepeatZ;

			glTexCoord2f(texCoordX1, texCoordZ1);
			glVertex3f(xPos, 0.0f, zPos1); // Bottom row of quads

			glTexCoord2f(texCoordX2, texCoordZ2);
			glVertex3f(xPos, 0.0f, zPos2); // Top row of quads
		}
		glEnd();
	}

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void AddTexture(char* fileName, GLuint& textureObj)
{
	// Adjust the size based on your longest path
	int width, height;

	char file[256]; // Adjust the size based on your longest path
	snprintf(file, sizeof(file), "./textures/%s", fileName);
	unsigned char* texture = BmpToTexture(file, &width, &height);
	if (texture == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file, width, height);

	glGenTextures(1, &textureObj);
	glBindTexture(GL_TEXTURE_2D, textureObj);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

}

void InitTextures()
{
	AddTexture("grass.bmp", GrassTexture);
	AddTexture("building_1.bmp", BuildingTexture[0]);
	AddTexture("building_2.bmp", BuildingTexture[1]);
	AddTexture("building_3.bmp", BuildingTexture[2]);
	AddTexture("building_4.bmp", BuildingTexture[3]);
	AddTexture("building_5.bmp", BuildingTexture[4]);
	AddTexture("building_6.bmp", BuildingTexture[5]);
	AddTexture("road.bmp", RoadTexture);
	AddTexture("lamp_post.bmp", LightPostTexture);
	AddTexture("stop_sign.bmp", StopSignTexture);
	AddTexture("side_walk.bmp", SideWalkTexture);
	AddTexture("tree_bark.bmp", TreeTexture);
	AddTexture("rust_car.bmp", CarTexture[0]);
	AddTexture("car_1.bmp", CarTexture[1]);
	AddTexture("car_2.bmp", CarTexture[2]);
	AddTexture("car_3.bmp", CarTexture[3]);
	AddTexture("bridge.bmp", BridgeTexture);
	AddTexture("car_2.bmp", TrainTexture);
}

void InitCarAnimations()
{
	car[0].xPos.Init();
	car[0].xPos.AddTimeValue(0.0, -40);
	car[0].xPos.AddTimeValue(5, 0);
	car[0].xPos.AddTimeValue(10, 40);

	car[1].xPos.Init();
	car[1].xPos.AddTimeValue(0.0, 40);
	car[1].xPos.AddTimeValue(5, 0);
	car[1].xPos.AddTimeValue(10, -40);

	car[2].xPos.Init();
	car[2].xPos.AddTimeValue(0.0, -40);
	car[2].xPos.AddTimeValue(5, 10);
	car[2].xPos.AddTimeValue(10, 40);

	car[3].xPos.Init();
	car[3].xPos.AddTimeValue(0.0, -50);
	car[3].xPos.AddTimeValue(5, 0);
	car[3].xPos.AddTimeValue(10, 30);

	car[4].xPos.Init();
	car[4].xPos.AddTimeValue(0.0, 30);
	car[4].xPos.AddTimeValue(5, 0);
	car[4].xPos.AddTimeValue(10, -40);

	car[5].zPos.Init();
	car[5].zPos.AddTimeValue(0.0, -30);
	car[5].zPos.AddTimeValue(3, -5);
	car[5].zPos.AddTimeValue(4, -5);
	car[5].zPos.AddTimeValue(6, 15);
	car[5].zPos.AddTimeValue(8, 15);
	car[5].zPos.AddTimeValue(10, 30);


	car[6].zPos.Init();
	car[6].zPos.AddTimeValue(0.0, 25);
	car[6].zPos.AddTimeValue(3, 5);
	car[6].zPos.AddTimeValue(4, 5);
	car[6].zPos.AddTimeValue(10, -30);

	TrainXPos.Init();
	TrainXPos.AddTimeValue(0.0, 0);
	TrainXPos.AddTimeValue(10.0, 62);
}


void AnimateCars()
{
	const int MSEC = 10000;		
	int msec = glutGet(GLUT_ELAPSED_TIME) % MSEC;

	float nowTime = (float)msec / 1000.;
	glEnable(GL_TEXTURE_2D);
	for (size_t i = 0; i < 5; i++)
	{
		glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, CarTexture[i%4]);
		glColor3f(1, 0, 0);
		glTranslatef(car[i].xPos.GetValue(nowTime), 0.5, 0.);
		glCallList(car[i].displayList);

		glPopMatrix();
	}

	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, CarTexture[3]);
	glTranslatef(0., 0.5, car[5].zPos.GetValue(nowTime));
	glColor3f(0, 1, 0);
	glCallList(car[5].displayList);

	glPopMatrix();
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, CarTexture[2]);
	glColor3f(0, 0, 1);
	glTranslatef(0., 0.5, car[6].zPos.GetValue(nowTime));
	glCallList(car[6].displayList);
	glPopMatrix();

	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, TrainTexture);
	glTranslatef(TrainXPos.GetValue(nowTime), 0,0 );
	glCallList(TrainList);
	glPopMatrix();
}

void InitLights()
{
	glEnable(GL_TEXTURE_2D);
	LightsList = glGenLists(1);
	glNewList(LightsList, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, LightPostTexture);
	glPushMatrix();
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	//upper road light
	DrawLampPost(30, 3.25, GL_LIGHT1);
	DrawLampPost(0, 3.25, GL_LIGHT2);
	DrawLampPost(-32.5, 3.25, GL_LIGHT3);
	//lower road light
	DrawLampPost(30, -16.25, GL_LIGHT4);
	DrawLampPost(0, -16.25, GL_LIGHT5);
	DrawLampPost(-32.5, -16.25, GL_LIGHT6);

	//vert road
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	DrawLampPost(-15,-15, GL_LIGHT7);
	
	glPopMatrix();
	glEndList();
	glDisable(GL_TEXTURE_2D);
}
void InitGround()
{
	GroundList = glGenLists(1);
	glNewList(GroundList, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, GrassTexture);
	glPushMatrix();
	DrawPlane(0,-.1, 0, 85, 60,1,1);
	glPopMatrix();
	glEndList();
}
void InitRoad()
{
	RoadList = glGenLists(1);
	glNewList(RoadList, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, RoadTexture);
	glPushMatrix();

	//draw upper road & side walks
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	DrawPlane(0,0,0,6,85,1,8);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, SideWalkTexture);
	DrawPlane(3.5, 0, -13.5, 1, 58.5, 1, 50);
	DrawPlane(-3.5, 0, -13.5, 1, 58.5, 1, 50);
	DrawPlane(3.5, 0, 32.75, 1, 20, 1, 16);
	DrawPlane(-3.5, 0, 32.75, 1, 20, 1, 16);

	//draw lower road & side walks
	glBindTexture(GL_TEXTURE_2D, RoadTexture);
	DrawPlane(-20, 0, 0, 6, 85, 1, 8);
	glBindTexture(GL_TEXTURE_2D, SideWalkTexture);
	DrawPlane(-15, 0, -13.5, 1, 58.5, 1, 50);
	DrawPlane(-24.5, 0, -13.5, 1, 58.5, 1, 50);
	DrawPlane(-15, 0, 32.75, 1, 20, 1, 16);
	DrawPlane(-24.5, 0, 32.75, 1, 20, 1, 16);

	//draw right vertical road
	glBindTexture(GL_TEXTURE_2D, RoadTexture);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	DrawPlane(-19.75, 0, 0, 6, 60, 1, 8);
	glBindTexture(GL_TEXTURE_2D, SideWalkTexture);
	//left side sidewalks
	DrawPlane(-16.25, 0, 16.5, 1, 27, 1, 30);
	DrawPlane(-16.25, 0, -9.25, 1, 12.5, 1, 10);
	DrawPlane(-16.25, 0, -26.5, 1, 7, 1, 6);
	//right side sidewalks
	DrawPlane(-23.25, 0, 17, 1, 26, 1, 30);
	DrawPlane(-23.25, 0, -9.25, 1, 12.5, 1, 10);
	DrawPlane(-23.25, 0, -26.5, 1, 7, 1, 6);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glEndList();
	
}

void InitSigns()
{
	glEnable(GL_TEXTURE_2D);
	SignList = glGenLists(1);
	glNewList(SignList, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, StopSignTexture);
	glPushMatrix();
	DrawSign(14, 1);
	DrawSign(14, 21);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	DrawSign(-25.5, .5);
	DrawSign(-25.5, -19);
	glPopMatrix();
	glEndList();
	glDisable(GL_TEXTURE_2D);
}

void DrawSign(float x, float z)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(x, 1, z);
	glScalef(10, 10, 10);
	LoadObjFile((char*)"./objects/stop_sign.obj");
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void DrawBuildings(float x, float z, float width, float height)
{
	
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	float baseY = -0.1f;

	
	SetMaterial(0.8f, 0.7f, 0.7f, 50.0f);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	// Front face
	glNormal3f(0.0f, 0.0f, -1.0f);  
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x - halfWidth, baseY, z - halfWidth);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + halfWidth, baseY, z - halfWidth);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + halfWidth, baseY + height, z - halfWidth);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x - halfWidth, baseY + height, z - halfWidth);

	// Back face
	glNormal3f(0.0f, 0.0f, 1.0f); 
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x - halfWidth, baseY, z + halfWidth);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + halfWidth, baseY, z + halfWidth);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + halfWidth, baseY + height, z + halfWidth);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x - halfWidth, baseY + height, z + halfWidth);

	// Left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x - halfWidth, baseY, z - halfWidth);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x - halfWidth, baseY, z + halfWidth);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x - halfWidth, baseY + height, z + halfWidth);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x - halfWidth, baseY + height, z - halfWidth);

	// Right face
	glNormal3f(1.0f, 0.0f, 0.0f);  
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + halfWidth, baseY, z - halfWidth);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + halfWidth, baseY, z + halfWidth);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + halfWidth, baseY + height, z + halfWidth);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + halfWidth, baseY + height, z - halfWidth);

	// Top face
	glNormal3f(0.0f, 1.0f, 0.0f); 
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x - halfWidth, baseY + height, z - halfWidth);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + halfWidth, baseY + height, z - halfWidth);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + halfWidth, baseY + height, z + halfWidth);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x - halfWidth, baseY + height, z + halfWidth);

	// Bottom face
	glNormal3f(0.0f, -1.0f, 0.0f); 
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x - halfWidth, baseY, z - halfWidth);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + halfWidth, baseY, z - halfWidth);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + halfWidth, baseY, z + halfWidth);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x - halfWidth, baseY, z + halfWidth);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	
}
void InitBuildings()
{
	BuildingsList = glGenLists(1);
	glNewList(BuildingsList, GL_COMPILE);

	//top row of buildings
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[1]);
	DrawBuildings(40, -6.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[3]);
	DrawBuildings(35, -6.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(30, -6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[2]);
	DrawBuildings(10, -6.5, 5, 12);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[4]);
	DrawBuildings(5, -6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(0, -6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[5]);
	DrawBuildings(-5, -6.5, 5, 10);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(-10, -6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[1]);
	DrawBuildings(-15, -6.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[3]);
	DrawBuildings(-20, -6.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(-25, -6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[2]);
	DrawBuildings(-30, -6.5, 5, 12);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[4]);
	DrawBuildings(-35, -6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[5]);
	DrawBuildings(-40, -6.5, 5, 10);
	
	//middle top buildings
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[5]);
	DrawBuildings(40, 6.5, 5, 10);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[4]);
	DrawBuildings(35, 6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[1]);
	DrawBuildings(30, 6.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[3]);
	DrawBuildings(10, 6.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[1]);
	DrawBuildings(5, 6.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(0, 6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[2]);
	DrawBuildings(-5, 6.5, 5, 12);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[4]);
	DrawBuildings(-10,6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[1]);
	DrawBuildings(-15, 6.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(-20, 6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[3]);
	DrawBuildings(-25, 6.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[4]);
	DrawBuildings(-30, 6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(-35, 6.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[2]);
	DrawBuildings(-40, 6.5, 5, 12);

	//middle bottom
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[3]);
	DrawBuildings(40, 12, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(35, 12, 5, 10);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[2]);
	DrawBuildings(30, 12, 5, 12);

	glBindTexture(GL_TEXTURE_2D, BuildingTexture[5]);
	DrawBuildings(10, 12, 5, 10);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[4]);
	DrawBuildings(5, 12, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[3]);
	DrawBuildings(0, 12, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[5]);
	DrawBuildings(-5, 12, 5, 10);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[2]);
	DrawBuildings(-10, 12, 5, 12);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[1]);
	DrawBuildings(-15, 12, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(-20, 12, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[2]);
	DrawBuildings(-25, 12, 5, 12);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[3]);
	DrawBuildings(-30, 12, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[4]);
	DrawBuildings(-35, 12, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[5]);
	DrawBuildings(-40, 12, 5, 10);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[1]);
	DrawBuildings(-40, 27.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[2]);
	DrawBuildings(-35, 27.5, 5, 10);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[3]);
	DrawBuildings(-30, 27.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[4]);
	DrawBuildings(-25, 27.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(-20, 27.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[2]);
	DrawBuildings(-15, 27.5, 5, 12);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[4]);
	DrawBuildings(-10, 27.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[5]);
	DrawBuildings(-5, 27.5, 5, 10);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[1]);
	DrawBuildings(-0, 27.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[0]);
	DrawBuildings(5, 27.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[4]);
	DrawBuildings(10, 27.5, 5, 9);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[5]);
	DrawBuildings(30, 27.5, 5, 10);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[1]);
	DrawBuildings(35, 27.5, 5, 7);
	glBindTexture(GL_TEXTURE_2D, BuildingTexture[2]);
	DrawBuildings(40, 27.5, 5, 10);
	glPopMatrix();
	glEndList();
}
void DrawLampPost(float x, float z,int num)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(x, 0, z);
	glScalef(10, 10, 10);
	LoadObjFile((char*)"./objects/Street_Light1.obj");

	glPopMatrix();
	glPushMatrix();

	SetSpotLight(num, x, 5,z- 1.5,   
		0.0f, -1.0f, 0.0f,  
		1, 1, 1);  
	glutSolidSphere(0.1f, 10, 10);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	
}
void InitBridge()
{
	BridgeList = glGenLists(1);
	glNewList(BridgeList, GL_COMPILE);
	glPushMatrix();
	glScalef(.5, .4, .4);
	glBindTexture(GL_TEXTURE_2D, BridgeTexture);
	DrawBridge(-35, -50);
	DrawBridge(25, -50);
	DrawBridge(-95, -50);
	glPopMatrix();
	glEndList();
}

void DrawBridge(float x, float z)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(x, 12, z);
	LoadObjFile((char*)"./objects/bridge.obj");
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void InitTrain()
{
	glEnable(GL_TEXTURE_2D);
	TrainList = glGenLists(1);
	glNewList(TrainList, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, TrainTexture);
	glPushMatrix();
	DrawTrain(-25, -20);
	glPopMatrix();
	glEndList();
	glDisable(GL_TEXTURE_2D);
}

void DrawTrain(float x, float z)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(x, 4.75, z);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(.4, .4, .4);
	LoadObjFile((char*)"./objects/train.obj");
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void InitVegetation()
{
	VegetationList = glGenLists(1);
	glNewList(VegetationList, GL_COMPILE);
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, TreeTexture);
	glColor3f(1, 1, 1);
	glRotatef(270.0f, 0.0f, 1.0f, 0.0f);

	//lower road veg
	DrawVegetation(16.25, 40);
	DrawVegetation(16.25, 20);
	DrawVegetation(16.25, 10);
	DrawVegetation(16.25, -10);
	DrawVegetation(16.25, -25);
	DrawVegetation(16.25, -40);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	DrawVegetation(-23.5, 10);
	DrawVegetation(-23.5, 0);
	DrawVegetation(-23.5, -10);
	DrawVegetation(-23.5, -20);
	DrawVegetation(-23.5, -30);
	DrawVegetation(-23.5, -40);
	DrawVegetation(-23.5, 25);
	DrawVegetation(-23.5, 32.5);
	DrawVegetation(-23.5, 37.5);
	
	glPopMatrix();
	//vert road veg
	glPushMatrix();
	DrawVegetation(15, -5);
	DrawVegetation(15, -25);
	DrawVegetation(15, 10);
	DrawVegetation(15, 27.5);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	DrawVegetation(-25, 25);
	DrawVegetation(-25, 15);
	DrawVegetation(-25, 5);
	DrawVegetation(-25, -10);
	DrawVegetation(-25, -27.5);
	glPopMatrix();
	glEndList();
}
void DrawVegetation(float x, float z)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(x, 0, z);
	glScalef(10, 10, 10);
	LoadObjFile((char*)"./objects/tree.obj");
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}
void InitCars()
{
	car[0].displayList = glGenLists(1);
	glNewList(car[0].displayList, GL_COMPILE);
	glPushMatrix();
	glScalef(.2, .2, .2);
	glColor3f(1, 0, 0);
	glTranslatef(0, 0, 7);
	LoadObjFile((char*)"./objects/car.obj");
	glPopMatrix();
	glEndList();

	car[1].displayList = glGenLists(1);
	glNewList(car[1].displayList, GL_COMPILE);
	glPushMatrix();
	glScalef(.2, .2, .2);
	glColor3f(0, 1, 0);
	glTranslatef(0, 0, -7);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	LoadObjFile((char*)"./objects/car.obj");
	glPopMatrix();
	glEndList();

	car[2].displayList = glGenLists(1);
	glNewList(car[2].displayList, GL_COMPILE);
	glPushMatrix();
	glScalef(.2, .2, .2);
	glColor3f(0, 1, 0);
	glTranslatef(0, 0, 108);
	
	LoadObjFile((char*)"./objects/car.obj");
	glPopMatrix();
	glEndList();
	
	car[3].displayList = glGenLists(1);
	glNewList(car[3].displayList, GL_COMPILE);
	glPushMatrix();
	glScalef(.2, .2, .2);
	glColor3f(0, 1, 0);
	glTranslatef(-10, 0, 108);
	LoadObjFile((char*)"./objects/car.obj");
	glPopMatrix();
	glEndList();

	car[4].displayList = glGenLists(1);
	glNewList(car[4].displayList, GL_COMPILE);
	glPushMatrix();
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glScalef(.2, .2, .2);
	glColor3f(0, 1, 0);
	glTranslatef(-30, 0, -92);
	LoadObjFile((char*)"./objects/car.obj");
	glPopMatrix();
	glEndList();
	
	car[5].displayList = glGenLists(1);
	glNewList(car[5].displayList, GL_COMPILE);
	glPushMatrix();
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(.2, .2, .2);
	glColor3f(0, 1, 0);
	glTranslatef(0, 0, -92);
	LoadObjFile((char*)"./objects/car.obj");
	glPopMatrix();
	glEndList();

	car[6].displayList = glGenLists(1);
	glNewList(car[6].displayList, GL_COMPILE);
	glPushMatrix();
	glRotatef(-270.0f, 0.0f, 1.0f, 0.0f);
	glScalef(.2, .2, .2);
	glColor3f(0, 1, 0);
	glTranslatef(0, 0, 105);
	LoadObjFile((char*)"./objects/car.obj");
	glPopMatrix();
	glEndList();
}

// initialize the glui window:

void
InitMenus( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(float) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}

// Function to handle mouse motion
void mouseMotion(int x, int y) {
	camera.lookAround(x, y);
}

void update(int value) {
	static float lastTime = 0.0f;
	float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	float deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	// Update the camera movement based on user input
	camera.move(deltaTime, moveForward, moveBackward, strafeLeft, strafeRight, moveUp, moveDown);

	// Reset the flags after movement
	moveUp = false;
	moveDown = false;

	glutPostRedisplay();
	glutTimerFunc(16, update, 0);
}


// the keyboard callback:

void Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'w':
	case 'W':
		moveForward = true;
		printf("Move Forward\n");
		break;

	case 'a':
	case 'A':
		camera.rotate(-5.0f);  
		printf("Rotate Left\n");
		break;

	case 's':
	case 'S':
		moveBackward = true;
		printf("Move Backward\n");
		break;

	case 'd':
	case 'D':
		camera.rotate(5.0f);  
		printf("Rotate Right\n");
		break;

	case 'q':
	case 'Q':
		strafeRight = true;
		printf("Rotation Left\n");
		break;

	case 'e':
	case 'E':
		strafeLeft = true;  
		printf("Rotation Right\n");
		break;

	case 'o':
	case 'O':
		NowProjection = ORTHO;
		printf("Orthogonal Projection\n");
		break;

	case 'p':
	case 'P':
		NowProjection = PERSP;
		printf("Perspective Projection\n");
		break;

	case ' ':
		camera.position[1] += camera.speed; 
		printf("Move Up\n");
		break;

	case 'Z':  
	case 'z': 
		camera.position[1] -= camera.speed;  
		printf("Move Down\n");
		break;

	case 'v':
	case 'V':
	case ESCAPE:
		DoMainMenu(QUIT);  // will not return here
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display():
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void handleKeyUp(unsigned char key, int x, int y) {
	if (key == 'w') moveForward = false;
	if (key == 's') moveBackward = false;
	if (key == 'q') strafeRight = false;
	if (key == 'e') strafeLeft = false;
}

// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}




///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit( float v[3] )
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
