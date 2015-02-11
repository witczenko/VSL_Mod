//
// Lighthouse3D.com VS*L OpenGL Sample
//
// Loading and displaying a Textured Model
//
// Uses:
//  Assimp 3.0 library for model loading
//		http://assimp.sourceforge.net/
//  Devil for image loading
//		http://openil.sourceforge.net/
//  GLEW for OpenGL post 1.1 functions
//		http://glew.sourceforge.net/
//	TinyXML for font definition parsing
//		http://sourceforge.net/projects/tinyxml/
//
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
//
// If you do use it I would love to hear about it. 
// Just post a comment at Lighthouse3D.com

// Have Fun :-)

#include <math.h>
#include <fstream>
#include <map>
#include <string>
#include <vector>


// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// Use Very Simple Libs
#include "vslibs.h"


VSMathLib *vsml;
VSShaderLib program;
VSFontLib vsfl;
VSResModelLib myModel;
unsigned int aSentence, profileSentence;

// Query to track the number of primitives
// issued by the tesselation shaders
GLuint counterQ;
unsigned int primitiveCounter = 0;

// Camera Position
float camX = 0, camY = 0, camZ = 5;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 0.0f, beta = 0.0f;
float r = 5.0f;


//// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];



// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	ratio = (1.0f * w) / h;
	vsml->loadIdentity(VSMathLib::PROJECTION);
	vsml->perspective(53.13f, ratio, 0.1f, 10000.0f);
}


// ------------------------------------------------------------
//
// Render stuff
//
void renderScene(void) {

	{
		PROFILE("Frame");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set both matrices to the identity matrix
		vsml->loadIdentity(VSMathLib::VIEW);
		vsml->loadIdentity(VSMathLib::MODEL);

		// set camera
		vsml->lookAt(camX, camY, camZ, 0,0,0, 0,1,0);

		{
			PROFILE_GL("Render models");

			// set the shader to render models
			glUseProgram(program.getProgramIndex());
			// start counting primitives
			glBeginQuery(GL_PRIMITIVES_GENERATED, counterQ);
			// render array of models
			for (float x = -2.0f ; x < 3.0f ; x += 2.0f) {
				for (float z = -2.0f; z < 3.0f ; z += 2.0f) {
					vsml->pushMatrix(VSMathLib::MODEL);
					vsml->translate(VSMathLib::MODEL, x, 0.0f, z);
					myModel.render();
					vsml->popMatrix(VSMathLib::MODEL);
				}
			}
			// stop counting primitives
			glEndQuery(GL_PRIMITIVES_GENERATED);
		}

		// FPS computation and display
		frame++;
		myTime=glutGet(GLUT_ELAPSED_TIME);
		if (myTime - timebase > 1000) {
				sprintf(s,"FPS:%4.2f  Counter: %d",
					frame*1000.0/(myTime-timebase) , primitiveCounter);
			timebase = myTime;
			frame = 0;
			vsfl.prepareSentence(aSentence,s);
			glutSetWindowTitle(s);
		}

		// Display text info
		{
			PROFILE("Dump");
			// prepare sentence with profile info
			std::string s = VSProfileLib::DumpLevels();
			vsfl.prepareSentence(profileSentence, s);
			//set the shader for rendering the sentence
			glUseProgram(program.getProgramIndex());
			// render sentences
			vsfl.renderSentence(10,10,aSentence);
			vsfl.renderSentence(10, 30, profileSentence);

		}
		 //swap buffers
		{
			PROFILE("Swap");
			glutSwapBuffers();
		}
	} // end PROFILE("Frame")
	{
		PROFILE("Collect GL Queries Time");
		VSProfileLib::CollectQueryResults();
		glGetQueryObjectuiv(counterQ, GL_QUERY_RESULT, &primitiveCounter);
	}
}


// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch(key) {

		case 27:

			glutLeaveMainLoop();
			break;

		case 'z': r -= 0.1f;
				camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
				camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
				camY = r *   						     sin(beta * 3.14f / 180.0f);
				break;
		case 'x': r += 0.1f;
				camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
				camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
				camY = r *   						     sin(beta * 3.14f / 180.0f);
				break;
		case 'm': glEnable(GL_MULTISAMPLE); break;
		case 'n': glDisable(GL_MULTISAMPLE); break;
		case 'k': VSProfileLib::Reset(); break;
		case 'p': std::string s = VSProfileLib::DumpLevels();
				printf("%s\n", s.c_str());
				break;

	}
	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);

//  uncomment this if not using an idle func
//	glutPostRedisplay();
}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN)  {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed
void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX =  - xx + startX;
	deltaY =    yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;

		rAux = r;

		camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		camY = rAux *   						       sin(betaAux * 3.14f / 180.0f);
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);

		camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		camY = rAux *   						       sin(betaAux * 3.14f / 180.0f);
	}


//  uncomment this if not using an idle func
//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);

//  uncomment this if not using an idle func
//	glutPostRedisplay();
}


// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for fonts and models
	program.init();
	program.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/dirlightdiffambpix.vert");
	program.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/dirlightdiffambpix.frag");

	// set semantics for the shader variables
	program.setProgramOutput(0, "outputF");
	program.setVertexAttribName(VSShaderLib::VERTEX_COORD_ATTRIB, "position");
	program.setVertexAttribName(VSShaderLib::TEXTURE_COORD_ATTRIB, "texCoord");
	program.setVertexAttribName(VSShaderLib::NORMAL_ATTRIB, "normal");

	program.prepareProgram();

	VSGLInfoLib::getProgramInfo(program.getProgramIndex());
	VSGLInfoLib::getUniformsInfo(program.getProgramIndex());
	printf("%s\n", program.getAllInfoLogs().c_str());
	// set sampler uniform
	program.setUniform("texUnit", 0);

	return(1);
}



// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//


int init()
{
	// load models
	if (myModel.load("models/fonte-finallambert.dae")) {

		printf("%s\n",myModel.getInfo().c_str());

		// set the uniform block name for the models uniforms

		// some GL settings
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_MULTISAMPLE);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		// generate a query to count primitives
		glGenQueries(1,&counterQ);

		return true;
	}
	else {
		printf("%s\n",myModel.getErrors().c_str());
		return false;
	}
}


void initVSL() {

	// set the material's block name
	VSResourceLib::setMaterialBlockName("Material");

	// Init VSML
	vsml = VSMathLib::getInstance();
	vsml->setUniformBlockName("Matrices");	
	vsml->setUniformName(VSMathLib::PROJ_VIEW_MODEL, "projViewModelMatrix");
	vsml->setUniformName(VSMathLib::NORMAL, "normalMatrix");

	// Init VSFL Fonts
	vsfl.load("fonts/couriernew10");
	vsfl.setFixedFont(true);
	vsfl.setColor(1.0f, 0.5f, 0.25f, 1.0f);
	aSentence = vsfl.genSentence();
	profileSentence = vsfl.genSentence();
}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char **argv) {

//  GLUT initialization
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA|GLUT_MULTISAMPLE);

	// Set context
	glutInitContextVersion (4, 4);
	glutInitContextProfile (GLUT_CORE_PROFILE );

	glutInitWindowPosition(100,100);
	glutInitWindowSize(640,360);
	glutCreateWindow("Lighthouse3D - VSL Demo");


//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc ( mouseWheel ) ;

//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();
	if (!glewIsSupported("GL_VERSION_4_4")) {
		printf("OpenGL 3.3 not supported\n");
		exit(1);
	}

	// this function will display generic OpenGL information
	VSGLInfoLib::getGeneralInfo();

	setupShaders();

	// init OpenGL and load model
	if (!init()) {
		printf("Could not Load the Model\n");
		exit(1);
	}

	initVSL();

	//  GLUT main loop
	glutMainLoop();

	return(1);

}

