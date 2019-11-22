//------------------------------------------------------------------------------
#include "SocketComm.h"
//------------------------------------------------------------------------------
#include <assert.h>
#include <math.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
//------------------------------------------------------------------------------
#include "chai3d.h"
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------
#ifndef MACOSX
#include "GL/glut.h"
#else
#include "GLUT/glut.h"
#endif
//#include "GL/glui.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled 
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;


//--------------------------------------------------------------------------
// VARIABLES SETTING FOR NETWORKING
//--------------------------------------------------------------------------

// make socket class with buffer size of 10
SocketComm sc(10);

// value for saving received data from client
char recogResult;

// initialize the socket with port number
int initResult = sc.Init(8888);

// set filename by received data from client
string filename1, filename2;

// use for label text
string str;


//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a light source to illuminate the objects in the world
cSpotLight *light;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// a virtual tool representing the haptic device in the scene
cToolCursor* tool;

// a few mesh objects
cMesh* object;
// cMesh* object1;
// cMesh* object2;
// cMesh* object3;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelHapticRate;

// indicates if the haptic simulation currently running
bool simulationRunning = false;

// indicates if the haptic simulation has terminated
bool simulationFinished = true;

// frequency counter to measure the simulation haptic rate
cFrequencyCounter frequencyCounter;

// information about computer screen and GLUT display window
int screenW;
int screenH;
int windowW;
int windowH;
int windowPosX;
int windowPosY;

// root resource path
string resourceRoot;

// calc max stifness
double maxStiffness;
//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------

// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void resizeWindow(int w, int h);

// callback when a key is pressed
void keySelect(unsigned char key, int x, int y);

// callback to render graphic scene
void updateGraphics(void);

// callback of GLUT timer
void graphicsTimer(int data);

// function that closes the application
void close(void);

// main haptics simulation loop
void updateHaptics(void);

// for networking, make a server and get data from user
void receiveData();



//==============================================================================
/*
    DEMO:   14-textures.cpp

    This example illustrates the use of haptic textures projected onto mesh
    surfaces.
*/
//==============================================================================

int main(int argc, char* argv[])
{
	// close the program if initializing socket has error
	if (initResult == 0) {
		exit(0);
	}

    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "Receive recognized information from deep learning server" << endl;
	cout << "And make a surface texture of it!" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[f] - Enable/Disable full screen mode" << endl;
    cout << "[x] - Exit application" << endl;
    cout << endl << endl;

    // parse first arg to try and locate resources
    resourceRoot = string(argv[0]).substr(0,string(argv[0]).find_last_of("/\\")+1);


    //--------------------------------------------------------------------------
    // OPEN GL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLUT
    glutInit(&argc, argv);

    // retrieve  resolution of computer display and position window accordingly
    screenW = glutGet(GLUT_SCREEN_WIDTH);
    screenH = glutGet(GLUT_SCREEN_HEIGHT);
    windowW = 0.8 * screenH;
    windowH = 0.5 * screenH;
    windowPosY = (screenH - windowH) / 2;
    windowPosX = windowPosY; 

    // initialize the OpenGL GLUT window
    glutInitWindowPosition(windowPosX, windowPosY);
    glutInitWindowSize(windowW, windowH);

    if (stereoMode == C_STEREO_ACTIVE)
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STEREO);
    else
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    // create display context and initialize GLEW library
    glutCreateWindow(argv[0]);

#ifdef GLEW_VERSION
    // initialize GLEW
    glewInit();
#endif

    // setup GLUT options
    glutDisplayFunc(updateGraphics);
    glutKeyboardFunc(keySelect);
    glutReshapeFunc(resizeWindow);
    glutSetWindowTitle("RENDERING SURFACE TEXTURE BY RECOGNITION");


    // set fullscreen mode
    if (fullscreen)
    {
        glutFullScreen();
    }


    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    world->m_backgroundColor.setBlack();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set(cVector3d(0.0, 0.0, 1.0),    // camera position (eye)
                cVector3d(0.0, 0.0, 0.0),    // lookat position (target)
                cVector3d(0.0, 1.0, 0.0));   // direction of the (up) vector

    // set the near and far clipping planes of the camera
    // anything in front or behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.01, 10.0);

    // set stereo mode
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.02);
    camera->setStereoFocalLength(1.0);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // create a light source
    light = new cSpotLight(world);

    // attach light to camera
    world->addChild(light);    

    // enable light source
    light->setEnabled(true);                   

    // position the light source
    light->setLocalPos(0.0, 0.0, 0.7);             

    // define the direction of the light beam
    light->setDir(0.0, 0.0, -1.0);             

    // enable this light source to generate shadows
    light->setShadowMapEnabled(true);       

    // set the resolution of the shadow map
    light->m_shadowMap->setQualityLow();
    //light->m_shadowMap->setQualityMedium();

    // set light cone half angle
    light->setCutOffAngleDeg(40);


    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // create a 3D tool and add it to the world
    tool = new cToolCursor(world);
    camera->addChild(tool);

    // position tool in respect to camera
    tool->setLocalPos(-1.0, 0.0, 0.0);

    // connect the haptic device to the tool
    tool->setHapticDevice(hapticDevice);

    // set radius of tool
    double toolRadius = 0.01;

    // define a radius for the tool
    tool->setRadius(toolRadius);

    // map the physical workspace of the haptic device to a larger virtual workspace.
    tool->setWorkspaceRadius(1.0);

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when 
    // the tool is located inside an object for instance. 
    tool->setWaitForSmallForce(true);

    // start the haptic tool
    tool->start();

	//--------------------------------------------------------------------------
	// CREATE OBJECTS
	//--------------------------------------------------------------------------

	// read the scale factor between the physical workspace of the haptic
	// device and the virtual workspace defined for the tool
	double workspaceScaleFactor = tool->getWorkspaceScaleFactor();

	// properties
	maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;

	// create a mesh
	object = new cMesh();

	// create plane
	cCreatePlane(object, 0.7, 0.7);

	// create collision detector
	object->createAABBCollisionDetector(toolRadius);

	// add object to world
	world->addChild(object);

	// set the position of the object
	object->setLocalPos(0.0, 0.0, 0.0);

	// set graphic properties

	/////////////////////////////////////////////////////////////////////////
	// DEFAULT OBJECT:
	/////////////////////////////////////////////////////////////////////////
	bool fileload;
	object->m_texture = cTexture2d::create();
	filename1 = "../resources/images/canvas.jpg";
	filename2 = "../../../bin/resources/images/canvas.jpg";

	// load file from location on filename
	fileload = object->m_texture->loadFromFile(RESOURCE_PATH(filename1.data()));
	if (!fileload)
	{
#if defined(_MSVC)
		fileload = object->m_texture->loadFromFile(filename2.data());
#endif
	}
	if (!fileload)
	{
		cout << "Error - Texture image failed to load correctly." << endl;
		close();
		return (-1);
	}

	// enable texture mapping
	object->setUseTexture(true);
	object->m_material->setWhite();


	// create normal map from texture data
	cNormalMapPtr normalMap = cNormalMap::create();
	normalMap->createMap(object->m_texture);
	object->m_normalMap = normalMap;


	// set haptic properties
	object->m_material->setStiffness(0.1 * maxStiffness);
	object->m_material->setStaticFriction(0.1);
	object->m_material->setDynamicFriction(0.1);
	object->m_material->setTextureLevel(0.3);
	object->m_material->setHapticTriangleSides(true, false);

	// before get result from server, render nothing
	object->setEnabled(false);


	//--------------------------------------------------------------------------
	// WIDGETS
	//--------------------------------------------------------------------------

	// create a font
	cFont *font = NEW_CFONTCALIBRI20();

	// create a label to display the haptic rate of the simulation
	labelHapticRate = new cLabel(font);
	camera->m_frontLayer->addChild(labelHapticRate);

	// create a background
	cBackground* background = new cBackground();
	camera->m_backLayer->addChild(background);

	// set background properties
	background->setCornerColors(cColorf(0.3, 0.3, 0.3),
		cColorf(0.2, 0.2, 0.2),
		cColorf(0.1, 0.1, 0.1),
		cColorf(0.0, 0.0, 0.0));

	// show data
	labelHapticRate->setText("Waiting...");

	//--------------------------------------------------------------------------
	// START SIMULATION
	//--------------------------------------------------------------------------

	// create a thread which starts the main haptics rendering loop
	cThread* hapticsThread = new cThread();
	hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

	cThread* networkThread = new cThread();
	networkThread->start(receiveData, CTHREAD_PRIORITY_GRAPHICS);

	// setup callback when application exits
	atexit(close);

	// start the main graphics rendering loop
//	glutTimerFunc(5000, receiveData, 0);
	glutTimerFunc(50, graphicsTimer, 0);
	glutMainLoop();


	// close the socket

	sc.Close();

	// exit
    return (0);
}

//------------------------------------------------------------------------------

void resizeWindow(int w, int h)
{
    windowW = w;
    windowH = h;
}

//------------------------------------------------------------------------------

void keySelect(unsigned char key, int x, int y)
{
    if ((key == 27) || (key == 'x'))
    {
        // exit application
        exit(0);
    }

    // option f: toggle fullscreen
    if (key == 'f')
    {
        if (fullscreen)
        {
            windowPosX = glutGet(GLUT_INIT_WINDOW_X);
            windowPosY = glutGet(GLUT_INIT_WINDOW_Y);
            windowW = glutGet(GLUT_INIT_WINDOW_WIDTH);
            windowH = glutGet(GLUT_INIT_WINDOW_HEIGHT);
            glutPositionWindow(windowPosX, windowPosY);
            glutReshapeWindow(windowW, windowH);
            fullscreen = false;
        }
        else
        {
            glutFullScreen();
            fullscreen = true;
        }
    }

}

//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    tool->stop();
}

//------------------------------------------------------------------------------

void graphicsTimer(int data)
{
    if (simulationRunning)
    {
        glutPostRedisplay();
    }

    glutTimerFunc(50, graphicsTimer, 0);
}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
	/////////////////////////////////////////////////////////////////////
	// UPDATE WIDGETS
	/////////////////////////////////////////////////////////////////////

	// update position of label
	labelHapticRate->setLocalPos((int)(0.5 * (windowW - labelHapticRate->getWidth())), 15);
	
	
	/////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);

    // render world
    camera->renderView(windowW, windowH);

    // swap buffers
    glutSwapBuffers();

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error: " << gluErrorString(err) << endl;
}

//------------------------------------------------------------------------------

void updateHaptics(void)
{
    // simulation in now running
    simulationRunning  = true;
    simulationFinished = false;

    // main haptic simulation loop
    while(simulationRunning)
    {
        // compute global reference frames for each object
        world->computeGlobalPositions(true);

        // update position and orientation of tool
        tool->updateFromDevice();

        // compute interaction forces
        tool->computeInteractionForces();

        // send forces to haptic device
        tool->applyToDevice();

        // update frequency counter
        frequencyCounter.signal(1);
    }
    
    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------

void receiveData() {
	while (true) {
		if (sc.ReceiveDatafromServer() == 1) {
			sc.PrintData();

			object->setEnabled(true);

			char recogResult = sc.GetSerialData();
			bool fileload;
			switch (recogResult) {
				case 'a':
					// load file from location on filename
					// set file location
					filename1 = "../resources/images/aspalt_sample.jpg";
					filename2 = "../../../bin/resources/images/aspalt_sample.jpg";

					fileload = object->m_texture->loadFromFile(RESOURCE_PATH(filename1.data()));
					if (!fileload)
					{
					#if defined(_MSVC)
						fileload = object->m_texture->loadFromFile(filename2.data());
					#endif
					}
					if (!fileload)
					{
						cout << "Error - Texture image failed to load correctly." << endl;
						close();
						exit(0);
					}

					// set haptic properties
					object->m_material->setStiffness(0.7 * maxStiffness);
					object->m_material->setStaticFriction(0.4);
					object->m_material->setDynamicFriction(0.3);
					object->m_material->setTextureLevel(1.0);
					object->m_material->setHapticTriangleSides(true, false);


					labelHapticRate->setText("Aspalt");

					break;


				case 't':
					// load file from location on filename
					// set file location
					filename1 = "../resources/images/tree_sample.jpg";
					filename2 = "../../../bin/resources/images/tree_sample.jpg";

					fileload = object->m_texture->loadFromFile(RESOURCE_PATH(filename1.data()));
					if (!fileload)
					{
					#if defined(_MSVC)
						fileload = object->m_texture->loadFromFile(filename2.data());
					#endif
					}
					if (!fileload)
					{
						cout << "Error - Texture image failed to load correctly." << endl;
						close();
						exit(0);
					}

					// set haptic properties
					object->m_material->setStiffness(0.8 * maxStiffness);
					object->m_material->setStaticFriction(0.3);
					object->m_material->setDynamicFriction(0.2);
					object->m_material->setTextureLevel(1.0);
					object->m_material->setHapticTriangleSides(true, false);

					labelHapticRate->setText("Tree");

					break;


				case 'b':
					// load file from location on filename
					// set file location
					filename1 = "../resources/images/bread_sample.jpg";
					filename2 = "../../../bin/resources/images/bread_sample.jpg";

					fileload = object->m_texture->loadFromFile(RESOURCE_PATH(filename1.data()));
					if (!fileload)
					{
					#if defined(_MSVC)
						fileload = object->m_texture->loadFromFile(filename2.data());
					#endif
					}
					if (!fileload)
					{
						cout << "Error - Texture image failed to load correctly." << endl;
						close();
						exit(0);
					}

					// set haptic properties
					object->m_material->setStiffness(0.07 * maxStiffness);
					object->m_material->setStaticFriction(0.3);
					object->m_material->setDynamicFriction(0.2);
					object->m_material->setTextureLevel(0.9);
					object->m_material->setHapticTriangleSides(true, false);

					labelHapticRate->setText("Bread");

					break;


			}

			// update picture of object
			object->m_texture->markForDeleteAndUpdate();
			
			// enable texture mapping
			object->setUseTexture(true);
			object->m_material->setWhite();

			// create normal map from texture data
			cNormalMapPtr normalMap = cNormalMap::create();
			normalMap->createMap(object->m_texture);
			object->m_normalMap = normalMap;

			// update text label with result
			labelHapticRate->markForUpdate();

		}
	}

}

//------------------------------------------------------------------------------
