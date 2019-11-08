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
	Miles miles; // steven
	shared_ptr<vector<shared_ptr<Shape>>> milesWithoutHatShapes; // steven
	shared_ptr<vector<shared_ptr<Shape>>> milesWithHatShapes; // steven
	shared_ptr<vector<shared_ptr<Shape>>> subwayShapes; // steven
	shared_ptr<Shape> cube;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 milesgMin; // steven
	vec3 milesgMax; // steven

	vec3 subwaygMin; // steven
	vec3 subwaygMax; // steven
	vec3 subwayScale; // steven
	vec3 subwayShift; // steven

	// texture for skymap
	unsigned int cubeMapTexture;

	// trash
	vector<shared_ptr<Particle>> particles;

    int test = 0;
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
		else if (key == GLFW_KEY_V && action == GLFW_PRESS) // move down
		{
			test += 1;
		}
		else if (key == GLFW_KEY_X && action == GLFW_PRESS) // move down
		{
			test -= 1;
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
		//glClearColor(.12f, .34f, .56f, 1.0f);
		glClearColor(.0f, .0f, .07f, 1.0f);
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
			if (sqrt((pow(p->position.x - miles.getTranslateX(), 2)) + (pow(p->position.z - miles.getTranslateZ(), 2))) < 1) {
				if (miles.getTranslateX() < p->position.x) {
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
		positions.push_back(vec3(-1, -1, -6));
		positions.push_back(vec3(-1.5, -1, -7));
		positions.push_back(vec3(1, -1, -5.5));
		positions.push_back(vec3(-1, -1, -4));

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

		/* Initialize Miles */
		milesWithHatShapes = make_shared<vector<shared_ptr<Shape>>>();
		loadMultipleShapeMesh(milesWithHatShapes, &milesgMin, &milesgMax, resourceDirectory + "/MilesWithHat.obj");
		miles.initializeNext(milesWithHatShapes, &milesgMin, &milesgMax);
		
 		milesWithoutHatShapes = make_shared<vector<shared_ptr<Shape>>>();
		loadMultipleShapeMesh(milesWithoutHatShapes, &milesgMin, &milesgMax, resourceDirectory + "/MilesWithoutHat.obj");	
		miles.initialize(milesWithoutHatShapes, &milesgMin, &milesgMax);
		
		miles.initStartingPosition();

		subwayShapes = make_shared<vector<shared_ptr<Shape>>>();
		loadMultipleShapeMesh(subwayShapes, &subwaygMin, &subwaygMax, resourceDirectory + "/Sewer_Scene.obj");
		getShiftAndScale(&subwayShift, &subwayScale, &subwaygMin, &subwaygMax);
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
		Model->translate(vec3(4, -2, -3));
		Model->scale(vec3(2, 3, 2));
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

		/*
		0: sides
		1: top
		2: lower pipe side
		3: floor
		4: left pipe top
		5: 
		6:
		7: light covers
		8: lights
		9: right pipe
		10: top rail
		11: left weird thing
		12: under railroad 
		13: top railroad
		14: tracks
		*/
		/* Draw Subway */ 
		Model->pushMatrix();
		Model->loadIdentity();
		Model->translate(vec3(10, 200*subwayScale.y*(subwaygMax.y - subwaygMin.y)/2 -1.4, 0));
		Model->rotate(radians(0.f), vec3(0, 1, 0));
		Model->scale(vec3(200, 200, 300)*subwayScale);
		Model->translate(vec3(-1,-1,-1)*subwayShift);
		//SetMaterial(prog, 6);		
		int shapesSize = subwayShapes->size();
		for (int i = 0; i < shapesSize; ++i)
		{
			if (i == 1 || i == 3)
				SetMaterial(prog, 8);
			else if (i == 0)
				SetMaterial(prog, 9);
			else if (i == 8)
				SetMaterial(prog, 11);
			else
				SetMaterial(prog, 10);
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
			subwayShapes->at(i)->draw(prog);	
		}
		Model->popMatrix();
		/*
		Model->pushMatrix();
		Model->translate(vec3(0, 200*subwayScale.y*(subwaygMax.y - subwaygMin.y)/2 -2, -406));
		//Model->rotate(radians(-30.f), vec3(0, 1, 0));
		Model->scale(vec3(200, 200, 300)*subwayScale);
		Model->translate(vec3(-1,-1,-1)*subwayShift);
		//SetMaterial(prog, 6);		
		for (int i = 0; i < shapesSize; ++i)
		{
			if (i == 1 || i == 3)
				SetMaterial(prog, 8);
			else if (i == 0)
				SetMaterial(prog, 9);
			else if (i == 8)
				SetMaterial(prog, 11);
			else
				SetMaterial(prog, 10);
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
			subwayShapes->at(i)->draw(prog);	
		}
		Model->popMatrix();
		 Draw Subway */

		// Draw miles
		miles.draw(prog); // steven
		miles.updatePosition(); // steve
		
		//drawTrash(Model);
		//drawFloor(Model);
		prog->unbind();

		//draw the sky box
		/*
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
			//cube->draw(cubeProg);
		glDepthFunc(GL_LESS);
		Model->popMatrix();
		cubeProg->unbind();
		*/

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
