/*
 * Program 3 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"
#include "Draw.h"
#include "Miles.h"
#include "Particle.h"
#include "math.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"
#define PI 3.14159265

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> cubeProg;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;
	shared_ptr<vector<shared_ptr<Shape>>> milesShapes;
	shared_ptr<vector<shared_ptr<Shape>>> milesWithHatShapes;
	shared_ptr<vector<shared_ptr<Shape>>> shapes;
	shared_ptr<Shape> cube;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;
	vec3 gMax;
	vec3 tMin;
	vec3 tMax;
	float lightPosX;
	float testRotate = 0;
	float saveTestRotate = 0;
	float translateX = -4;
	float translateZ = -8;
	float tempCount = 0;
	float headSideToSide = 0;
	float bodySideToSide = 0;
	float pauses = 0;
	vec3 rotateMiles;
	vec3 rotateHead;
	float shoulderRotate = 0;
	float armRotate = 0;
	float legRotate = 0;
	vec3 L1S;
	vec3 L1K;
	vec3 R1K;
	vec3 R1S;

	int positionCount = 1;

	// texture for skymap
	unsigned int cubeMapTexture;

	// trash
	vector<shared_ptr<Particle>> particles;

	float beta = 0;
	float alpha = 0; 
	float speed = 0;
	int drawMode = 0;

	vec3 eye;
	vec3 LA;
	vec3 view;
	vec3 u;
	vec3 upVector;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
 			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			drawMode = 1;
 		}
 		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
 			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			drawMode = 0;
 		}
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if (key == GLFW_KEY_A && action == GLFW_PRESS) // move left
		{
			speed = -0.2;
			view = normalize(LA - eye);
			u = normalize(cross(view, upVector));
			eye = eye + vec3(speed)*u;
			LA = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(PI/2 - beta)) + eye;
		}
		else if (key == GLFW_KEY_D && action == GLFW_PRESS) // move right
		{
			speed = 0.2;
			view = normalize(LA - eye);
			u = normalize(cross(view, upVector));
			eye = eye + vec3(speed)*u;
			LA = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(PI/2 - beta)) + eye;
		}
		else if (key == GLFW_KEY_W && action == GLFW_PRESS) // move up
		{
		    speed = 0.2;
		    view = normalize(LA - eye);
		    eye = eye + vec3(speed)*view;
		    LA = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(PI/2 - beta)) + eye;
		}
		else if (key == GLFW_KEY_S && action == GLFW_PRESS) // move down
		{
			speed = -0.2;
			view = normalize(LA - eye);
			eye = eye + vec3(speed)*view;
		    LA = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(PI/2 - beta)) + eye;
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		beta -= (float) (deltaX/(float)width) * PI;
		alpha += (float) (deltaY/(float)height) * PI;
		if (alpha > radians(80.f))
			alpha = radians(80.f);
		else if (alpha < radians(-80.f))
			alpha = radians(-80.f);
		LA = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(PI/2 - beta)) + eye;
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	unsigned int createSky(string dir, vector<string> faces)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(false);
		for(GLuint i = 0; i < faces.size(); i++) {
    		unsigned char *data = 
			stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);
			if (data) {
    			glTexImage2D(
        			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
        			0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			} else {
				std::cout << "failed to load: " << (dir+faces[i]).c_str() << std::endl;
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  

		cout << " creating cube map any errors : " << glGetError() << endl;
		return textureID;
	}

	// Code to load in the textures
	void initTex(const std::string& resourceDirectory)
	{
		vector<std::string> faces {
    	"yoko_rt.jpg",
    	"yoko_lf.jpg",
    	"yoko_up.jpg",
    	"yoko_dn.jpg",
    	"yoko_ft.jpg",
    	"yoko_bk.jpg"
		}; 
		cubeMapTexture = createSky(resourceDirectory + "/yoko/",  faces);
	}

	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// camera variables
		alpha = 0;
		beta = -PI/2;
		eye = vec3(0, 0, 0);
		LA = vec3(0, 0, -1);
		view = normalize(LA - eye);
		upVector = vec3(0, 1 ,0);
		u = normalize(cross(view, upVector));

		// adnimation variables
		rotateMiles = vec3(0.6*PI, 0, 0);
		rotateHead = vec3(0, 0, PI/4);
		L1S = vec3(0);
		L1K = vec3(0);
		R1S = vec3(0);
		R1K = vec3(0);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
		if (! prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("M");
		prog->addUniform("V");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		prog->addUniform("lightDir");
		prog->addUniform("lightCol");
		prog->addUniform("eye");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		//initialize the textures we might use
		initTex(resourceDirectory);

		// skybox
		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames(
			resourceDirectory + "/cube_vert.glsl",
			resourceDirectory + "/cube_frag.glsl");
		if (! cubeProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
 		cubeProg->addUniform("P");
		cubeProg->addUniform("M");
		cubeProg->addUniform("V");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");
	 }

	void updateTrash() {
		for (shared_ptr<Particle> p : particles) {
			if (sqrt((pow(p->position.x - translateX, 2)) + (pow(p->position.z - translateZ, 2))) < 1) {
				if (translateX < p->position.x) {
					p->velocity = vec3(.1, 0, 0);
				}
				else {
					p->velocity = vec3(-.1, 0, 0);
				}
			}
			p->position = vec3(p->position.x + p->velocity.x, p->position.y + p->velocity.y, p->position.z + p->velocity.z);
		}
	}

	void initTrash() {
		vector<vec3> positions;
		positions.push_back(vec3(-1, -1, -7));
		positions.push_back(vec3(-1.5, -1, -8));
		positions.push_back(vec3(1, -1, -6.5));
		positions.push_back(vec3(-1, -1, -5));

		for (int i = 0; i < 4; i++) {
			shared_ptr<Particle> p = make_shared<Particle>();
			p->position = positions[i];
			p->velocity = vec3(0, 0, 0);
			particles.push_back(p);
		}
		shared_ptr<Particle> p = make_shared<Particle>();
	}

	void initGeom(const std::string& resourceDirectory)
	{
		cube = make_shared<Shape>();
		cube->loadSingleShapeMesh(resourceDirectory + "/cube.obj");
		//load in the mesh and make the shape(s)
 		milesShapes = make_shared<vector<shared_ptr<Shape>>>();
		loadMultipleShapeMesh(milesShapes, &gMin, &gMax, resourceDirectory + "/MilesWithoutHat.obj");	
		milesWithHatShapes = make_shared<vector<shared_ptr<Shape>>>();
		loadMultipleShapeMesh(milesWithHatShapes, &gMin, &gMax, resourceDirectory + "/MilesWithHat.obj");
	}

	void drawFloor(shared_ptr<MatrixStack> Model) {
		Model->pushMatrix();
		Model->translate(vec3(0, -2, 0));
		Model->scale(vec3(60, .05, 60));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		SetMaterial(prog, 2);
		cube->draw(prog);
		Model->popMatrix();
	}
	void drawTrash(shared_ptr<MatrixStack> Model) {
		//draw trash bin
		Model->pushMatrix();
		Model->translate(vec3(3, -2, -3));
		Model->scale(vec3(2, 4, 2));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		SetMaterial(prog, 4);
		cube->draw(prog);
		Model->popMatrix();

		updateTrash();
		//draw trash
		for (shared_ptr<Particle> p : particles) {
			
			Model->pushMatrix();
			Model->translate(p->position);
			Model->scale(vec3(.2, .2, .2));
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
			SetMaterial(prog, 5);
			cube->draw(prog);
			Model->popMatrix();
		}

	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);		

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Leave this code to just draw the meshes alone */
		float aspect = width/(float)height;

		// Create the matrix stacks
		auto P = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();
		// Apply perspective projection.
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 200.0f);

		mat4 V = lookAt(eye, LA, upVector);

		//Draw our scene - two meshes - right now to a texture
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(V));
		glUniform3f(prog->getUniform("lightDir"), 0.2, 1, 0.2);
		glUniform3f(prog->getUniform("lightCol"), 1, 1, 1);
		glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);

		// shift and scale of dummy after measurements
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
			Model->translate(vec3(translateX,-1, translateZ));
			Model->rotate(rotateMiles.x, vec3(1, 0, 0)); // axis left and right through body
			Model->rotate(rotateMiles.z, vec3(0, 0, 1));
			Model->rotate(PI/2 + testRotate, vec3(0, 1, 0)); // axis up and down through head
			Model->scale(scale);
			Model->translate(vec3(-1,-1,-1)*shift);
			SetMaterial(prog, 0);		
			int shapesSize = milesShapes->size();
			armRotate = sin(tempCount);
			shoulderRotate = cos(tempCount)/2;
			for (int i = 0; i < shapesSize; ++i)
			{
				// Color the eyes
				SetMaterial(prog, 0);
				if (i == 2 || i == 4)
					SetMaterial(prog, 2);
				else if (i == 3 || i == 5 || i == 38)
					SetMaterial(prog, 3);

				// Animate the parts
				if (!(isLeftLeg4(i) || isRightLeg4(i)))
				{
					vec3 bodyRotate = vec3(-0.2*abs(sin(bodySideToSide)), 0, 0);
					setupPart(milesShapes, Model, i, 0, &bodyRotate); 
				}
				if ((i >= 1 && i <= 5) || i == 38)
				{
					vec3 headRotate = vec3(rotateHead.x + sin(headSideToSide)/4, rotateHead.y + PI*sin(headSideToSide)/4, rotateHead.z);
					setupPart(milesShapes, Model, i, 1, &headRotate);
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
					milesShapes->at(i)->draw(prog);	
					Model->popMatrix();
				}
				else if (isLeftLeg1(i))
				{
					vec3 rotateL1S = vec3(shoulderRotate + L1S.x, -shoulderRotate + L1S.y, -shoulderRotate+L1S.z);
					vec3 rotateL1K = L1K;
					drawLeg(prog, milesShapes, Model, i, 25, &rotateL1S, 24, &rotateL1K);
				}
				else if (isLeftLeg2(i))
				{
					vec3 rotateL2S = vec3(0, -shoulderRotate/2, 0);
					vec3 rotateL2K = vec3(armRotate, 0, armRotate);
					drawLeg(prog, milesShapes, Model, i, 26, &rotateL2S, 23, &rotateL2K);
				}
				else if (isLeftLeg3(i))
				{
					vec3 rotateL3S = vec3(0, 0, shoulderRotate);
					vec3 rotateL3K = vec3(armRotate, -armRotate, 0);
					drawLeg(prog, milesShapes, Model, i, 27, &rotateL3S, 22, &rotateL3K);
				}
				else if (isLeftLeg4(i))
				{
					vec3 rotateL4S = vec3(0, 0, sin(legRotate)/4 - 0.4);
					vec3 rotateL4K = vec3(0, 0, sin(legRotate)/6 + 0.5);
					drawLeg(prog, milesShapes, Model, i, 28, &rotateL4S, 29, &rotateL4K);
				}
				else if (isRightLeg1(i))
				{
					vec3 rotateR1S = vec3(R1S.x, R1S.y+shoulderRotate/2, R1S.z);
					vec3 rotateR1K = vec3(R1K.x+-armRotate/2, R1K.y, R1K.z);
					drawLeg(prog, milesShapes, Model, i, 34, &rotateR1S, 35, &rotateR1K);
				}
				else if (isRightLeg2(i))
				{
					vec3 rotateR2S = vec3(0);
					vec3 rotateR2K = vec3(-armRotate, 0, 0);
					drawLeg(prog, milesShapes, Model, i, 33, &rotateR2S, 36, &rotateR2K);
				}
				else if (isRightLeg3(i))
				{
					vec3 rotateR3S = vec3(0, 0, shoulderRotate);
					vec3 rotateR3K = vec3(-armRotate, 0, 0);
					drawLeg(prog, milesShapes, Model, i, 32, &rotateR3S, 37, &rotateR3K);
				}
				else if (isRightLeg4(i))
				{
					vec3 rotateR4S = vec3(0, 0, sin(-legRotate)/4 - 0.4);
					vec3 rotateR4K = vec3(0, 0, sin(-legRotate)/6 + 0.5);
					drawLeg(prog, milesShapes, Model, i, 31, &rotateR4S, 30, &rotateR4K);
				}
				else
				{
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
					milesShapes->at(i)->draw(prog);	
				}
				if (!(isLeftLeg4(i) || isRightLeg4(i)))
				{
					Model->popMatrix();
				}
			}
		Model->popMatrix();
		
		if (positionCount == 1) // Roll Over
		{
			animatePart(&testRotate, 0, -4*PI, true, 60);
			animatePart(&translateX, -4, 0, true, 60);
			if (floatEqual(translateX, 0))
			{
				positionCount++;
				saveTestRotate = testRotate;
			}
		}
		else if (positionCount == 2) // Move legs while upside down
		{
			animatePart(&tempCount, 0, 10, true, 100);
			legRotate = tempCount;
			testRotate = (saveTestRotate + cos(tempCount)/3);
			if (floatEqual(tempCount, 10))
			{
				positionCount++;
				saveTestRotate = testRotate;
			}
		}
		else if (positionCount == 3) // Flip up and look left
		{
			animatePart(&testRotate, saveTestRotate, -3*PI, true, 20);
			animatePart(&rotateHead.z, PI/4, PI/2, testRotate < -2.9*PI, 90);
			animatePart(&pauses, 0, 10, rotateHead.z > PI/2.1, 100);
			animatePart(&rotateHead.y, 0, -PI/4, pauses > 9, 50);
			animatePart(&tempCount, 10, 5, pauses > 9, 50);
			legRotate = tempCount;
			animatePart(&rotateMiles.z, 0, PI/4, pauses>9, 50);
			if (floatEqual(tempCount, 5))
			{
				positionCount++;
			}
		}
		else if (positionCount == 4) // look right
		{
			animatePart(&pauses, 10, 0, true, 60);
			animatePart(&rotateHead.y, -PI/4, PI/8, pauses < 2, 50);
			animatePart(&tempCount, 5, 0, pauses < 2, 50);
			legRotate = tempCount;
			animatePart(&rotateMiles.z, PI/4, -PI/8, pauses < 2, 50);
			if (floatEqual(tempCount, 0))
			{
				positionCount++;
			}
		}
		else if (positionCount == 5) // move to trash and get hat
		{
			animatePart(&pauses, 0, 10, true, 60);
			animatePart(&translateX, 0, 3, pauses > 8, 50);
			animatePart(&tempCount, 0, 7, pauses > 7, 50);
			legRotate = tempCount;
			if (floatEqual(tempCount, 7))
			{
				positionCount++;
				milesShapes = milesWithHatShapes;
				testRotate = 3*PI/4;
				saveTestRotate = 0;
				tempCount = 0;
				legRotate = 0;
				translateX = 3;
				armRotate = 0;
				shoulderRotate = 0;
				tempCount = 0;
				rotateHead = vec3(0);
				rotateMiles = vec3(0);
			}
		}
		else if (positionCount == 6) // walk out on two legs
		{
			animatePart(&pauses, 10, 0, true, 400);
			animatePart(&tempCount, 0, 20, pauses < 5, 200);
			legRotate = tempCount;
			animatePart(&translateX, 3, 0, pauses < 5, 200);
			animatePart(&translateZ, -8, -4, pauses < 5, 200);
			animatePart(&testRotate, 3*PI/4, 3*PI, translateX < 1, 50);
			if (floatEqual(translateX, 0))
			{
				positionCount++;
			}
		}
		else if (positionCount == 7) // Put arm down and start spiderman dance
		{
			animatePart(&L1K.z, 0, -1.6, pauses > 8, 20);
			animatePart(&L1S.x, 0, -0.8, pauses > 8, 20);
			animatePart(&L1S.y, 0, -0.8, pauses > 8, 20);
			animatePart(&pauses, 0, 10, true, 50);
			animatePart(&tempCount, 20, 10, pauses > 8, 100);
			animatePart(&headSideToSide, 0, -1.6, pauses > 8, 50);
			animatePart(&bodySideToSide, 0, 10, pauses > 8, 100);
			if (floatEqual(tempCount, 10))
			{
				positionCount++;
			}
		}
		else if (positionCount == 8) // look the other way
		{
			animatePart(&headSideToSide, -1.6, -3*PI/2, true, 50);
			animatePart(&tempCount, 10, 0, true, 100);
			animatePart(&bodySideToSide, 10, 0, true, 100);
			if (floatEqual(tempCount, 0))
			{
				positionCount++;
			}
		}
		else if (positionCount == 9) // put hands up
		{
			animatePart(&headSideToSide, -3*PI/2, -1.6, true, 50);
			animatePart(&bodySideToSide, 0, 10, true, 50);
			animatePart(&L1K.z, -1.6, 0, true, 50);
			animatePart(&L1S.x, -0.8, 0, true, 50);
			animatePart(&L1S.y, -0.8, 0, true, 50);
			animatePart(&R1K.y, 0, -1, true, 50);
			animatePart(&R1K.z, 0, 1, true, 50);
			animatePart(&R1S.z, 0, 1, true, 50);
		}
		drawTrash(Model);
		drawFloor(Model);
		prog->unbind();

		//draw the sky box
		cubeProg->bind();
		glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
		glUniformMatrix4fv(cubeProg->getUniform("V"), 1, GL_FALSE, value_ptr(V));
		glDepthFunc(GL_LEQUAL);
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0, 5, 0));
			Model->translate(eye);
			Model->rotate(radians(-180.f), vec3(0, 1, 0));
			Model->scale(vec3(100));
			glUniformMatrix4fv(cubeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
			cube->draw(cubeProg);
		glDepthFunc(GL_LESS);
		Model->popMatrix();
		cubeProg->unbind();

		P->popMatrix();
		speed = 0;
	}	
};

int main(int argc, char **argv)
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
	windowManager->init(512, 512);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initTrash();

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
