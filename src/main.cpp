/*
 * Program 3 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Draw.h"
#include "Miles.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.14159265

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> shape;
	shared_ptr<vector<shared_ptr<Shape>>> milesShapes;
	shared_ptr<vector<shared_ptr<Shape>>> shapes;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;
	vec3 gMax;
	float lightPosX;
	float x = 0;
	float testRotate = 0;
	float armRotate = 0;
	int test = 0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			lightPosX -= 1;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS) {
			lightPosX += 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			test += 1;
		}
		if (key == GLFW_KEY_J && action == GLFW_PRESS) {
			armRotate += 0.1;
		}
		if (key == GLFW_KEY_K && action == GLFW_PRESS) {
			armRotate -= 0.1;
		}

	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		prog->addUniform("lightPos");
		prog->addUniform("lightCol");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
	}

	void initGeom(const std::string& resourceDirectory)
	{
		//load in the mesh and make the shape(s)
 		milesShapes = make_shared<vector<shared_ptr<Shape>>>();
		loadMultipleShapeMesh(milesShapes, &gMin, &gMax, resourceDirectory + "/MilesWithCylinders.obj");		
	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		// View is identity - for now
		View->pushMatrix();

		// Draw a stack of cubes with indiviudal transforms
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(prog->getUniform("lightPos"), lightPosX, 2, 2);
		glUniform3f(prog->getUniform("lightCol"), 1, 1, 1);

		
		vec3 shift;
		vec3 scale;
		getShiftAndScale(&shift, &scale, &gMin, &gMax);

		// 0 - body
		// 1 - head
		// 2 - LIEye
		// 3 - LOEye
		// 4 - RIEye
		// 5 - ROEye
		// 6 - L1I
		// 7 - L1O
		// 8 - L2I
		// 9 - L2O
		// 10 - L3I
		// 11 - L3O
		// 12 - R2O
		// 13 - R2I
		// 14 - R1I
		// 15 - R1O
		// 16 - R3O
		// 17 - R3I
		// 18 - L4I
		// 19 - L4O
		// 20 - R4O
		// 21 - R4I
		// 22 - L3E
		// 23 - L2E
		// 24 - L1E
		// 25 - L1S
		// 26 - L2S
		// 27 - L3S
		// 28 - L4S
		// 29 - L4E
		// 30 - R4E
		// 31 - R4S
		// 32 - R3S
		// 33 - R2S
		// 34 - R1S
		// 35 - R2E
		// 36 - R1E
		// 37 - R3E



		// Draw miles
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0,0,-3));
			Model->rotate(0.6*PI, vec3(1, 0, 0)); // axis left and right through body
			Model->rotate(PI/2 - testRotate, vec3(0, 1, 0)); // axis up and down through head
			Model->scale(scale);
			Model->translate(vec3(-1,-1,-1)*shift);
			SetMaterial(prog, 0);		
			int shapesSize = milesShapes->size();
			testRotate += 0.1;
			//armRotate += 0.1;
			for (int i = test; i < shapesSize; ++i)
			{
				// Color the eyes
				SetMaterial(prog, 0);
				if (i == 2 || i == 4)
					SetMaterial(prog, 2);
				else if (i == 3 || i == 5)
					SetMaterial(prog, 3);

				// Animate the parts
				if (i >= 1 && i <= 5)
				{
					vec3 headRotate = vec3(0, 0, PI/4);
					setupPart(milesShapes, Model, i, 1, &headRotate);
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
					milesShapes->at(i)->draw(prog);	
					Model->popMatrix();
				}
				else if (isLeftLeg1(i))
				{
					vec3 rotateL1S = vec3(0);
					vec3 rotateL1K = vec3(0);
					drawLeg(prog, milesShapes, Model, i, 25, &rotateL1S, 24, &rotateL1K);
					//x+=0.01;
				}
				else if (isLeftLeg2(i))
				{
					vec3 rotateL2S = vec3(0);
					vec3 rotateL2K = vec3(armRotate, 0, 0);
					drawLeg(prog, milesShapes, Model, i, 26, &rotateL2S, 23, &rotateL2K);
				}
				else if (isLeftLeg3(i))
				{
					vec3 rotateL3S = vec3(0);
					vec3 rotateL3K = vec3(armRotate, 0, 0);
					drawLeg(prog, milesShapes, Model, i, 27, &rotateL3S, 22, &rotateL3K);
				}
				else if (isLeftLeg4(i))
				{
					vec3 rotateL4S = vec3(0);
					vec3 rotateL4K = vec3(0);
					drawLeg(prog, milesShapes, Model, i, 28, &rotateL4S, 29, &rotateL4K);
				}
				else if (isRightLeg1(i))
				{
					vec3 rotateR1S = vec3(0);
					vec3 rotateR1K = vec3(armRotate, 0, 0);
					drawLeg(prog, milesShapes, Model, i, 34, &rotateR1S, 35, &rotateR1K);
				}
				else if (isRightLeg2(i))
				{
					vec3 rotateR2S = vec3(0);
					vec3 rotateR2K = vec3(armRotate, 0, 0);
					drawLeg(prog, milesShapes, Model, i, 33, &rotateR2S, 36, &rotateR2K);
				}
				else if (isRightLeg3(i))
				{
					vec3 rotateR3S = vec3(0);
					vec3 rotateR3K = vec3(-3.3+armRotate, 0, 0);
					drawLeg(prog, milesShapes, Model, i, 32, &rotateR3S, 37, &rotateR3K);
				}
				else if (isRightLeg4(i))
				{
					vec3 rotateR4S = vec3(0);
					vec3 rotateR4K = vec3(0.5+armRotate, 0, 0);
					drawLeg(prog, milesShapes, Model, i, 31, &rotateR4S, 30, &rotateR4K);
				}
				else
				{
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
					milesShapes->at(i)->draw(prog);	
				}
			}
		Model->popMatrix();

		prog->unbind();
		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();
	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
