#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Program.h"
#include "Shape.h"
#include "Draw.h"
#include "MatrixStack.h"

#define PI 3.14159265

using namespace glm;
using namespace std;

// sets material for the meshes
void SetMaterial(const shared_ptr<Program> prog, int i) 
{
	switch (i) 
	{
		case 0: // black
		glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.02, 0.02);
		glUniform3f(prog->getUniform("MatDif"), 0.1, 0.1, 0.1);
		glUniform3f(prog->getUniform("MatSpec"), 0.4, 0.4, 0.4);
		glUniform1f(prog->getUniform("shine"), 10.0);
		break;
		case 1: // flat grey
		glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
		glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
		glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);
		glUniform1f(prog->getUniform("shine"), 4.0);
		break;
		case 2: //white
		glUniform3f(prog->getUniform("MatAmb"), 0.3, 0.3, 0.3);
		glUniform3f(prog->getUniform("MatDif"), 0.55, 0.55, 0.55);
		glUniform3f(prog->getUniform("MatSpec"), 0.7, 0.7, 0.7);
		glUniform1f(prog->getUniform("shine"), 32.0);
		break;
		case 3: // ruby
		glUniform3f(prog->getUniform("MatAmb"), 0.1745, 0.01175, 0.01175);
		glUniform3f(prog->getUniform("MatDif"), 0.61424, 0.04136, 0.04136);
		glUniform3f(prog->getUniform("MatSpec"), 0.727811, 0.62695, 0.62695);
		glUniform1f(prog->getUniform("shine"), 76.8);
		break;
		case 4: // green
		glUniform3f(prog->getUniform("MatAmb"), 0.1745, 0.01175, 0.01175);
		glUniform3f(prog->getUniform("MatDif"), 0.1, 0.6, 0.1);
		glUniform3f(prog->getUniform("MatSpec"), 0.727811, 0.62695, 0.62695);
		glUniform1f(prog->getUniform("shine"), 76.8);
		break;
		case 5: // blue
		glUniform3f(prog->getUniform("MatAmb"), 0.05, 0.05, 0.2);
		glUniform3f(prog->getUniform("MatDif"), 0.0, 0.1, 0.9);
		glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);
		glUniform1f(prog->getUniform("shine"), 20.);
		break;
		case 6: // Material.010
		glUniform3f(prog->getUniform("MatAmb"), 1, 1, 1);
		glUniform3f(prog->getUniform("MatDif"), 0.147314, 0.069027, 0.031129);
		glUniform3f(prog->getUniform("MatSpec"), 0.500000, 0.500000, 0.500000);
		glUniform1f(prog->getUniform("shine"), 225);
		break;
		case 7: // Pipe
		glUniform3f(prog->getUniform("MatAmb"), 1.0, 1.0, 1.0);
		glUniform3f(prog->getUniform("MatDif"), 0.310813, 0.310813, 0.310813);
		glUniform3f(prog->getUniform("MatSpec"), 0.50000, 0.50000, 0.50000);
		glUniform1f(prog->getUniform("shine"), 90.249998);
		break;
		case 8: // Red Subway
		glUniform3f(prog->getUniform("MatAmb"), 0.15, 0.05, 0.05);
		glUniform3f(prog->getUniform("MatDif"), 0.426, 0.17, 0.18);
		glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.057, 0.05);
		glUniform1f(prog->getUniform("shine"), 90.249998);
		break;
		case 9: // Blue Subway
		glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.02, 0.2);
		glUniform3f(prog->getUniform("MatDif"), 0.1, 0.1, 0.2);
		glUniform3f(prog->getUniform("MatSpec"), 0.007, 0.009, 0.077);
		glUniform1f(prog->getUniform("shine"), 4);
		break;
		case 10: // flat grey Subway
		glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
		glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.3);
		glUniform3f(prog->getUniform("MatSpec"), 0.1, 0.1, 0.1);
		glUniform1f(prog->getUniform("shine"), 4.0);
		break;
		case 11: //white Subway
		glUniform3f(prog->getUniform("MatAmb"), 0.55, 0.55, 0.55);
		glUniform3f(prog->getUniform("MatDif"), 0.55, 0.55, 0.55);
		glUniform3f(prog->getUniform("MatSpec"), 0.7, 0.7, 0.7);
		glUniform1f(prog->getUniform("shine"), 32.0);
		break;
	}
}