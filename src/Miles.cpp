#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Program.h"
#include "Shape.h"
#include "Draw.h"
#include "MatrixStack.h"
#include "Miles.h"

#define PI 3.14159265

using namespace glm;
using namespace std;

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
		// 35 - R1E
		// 36 - R2E
		// 37 - R3E

bool isLeftLeg1(int i) { return (isLowerLeftLeg1(i) || i == 6 || i == 25); }
bool isLowerLeftLeg1(int i) { return (i == 7 || i == 24); }
bool isLeftLeg2(int i) { return (isLowerLeftLeg2(i) || i == 8 || i == 26); }
bool isLowerLeftLeg2(int i) { return (i == 9 || i == 23); }
bool isLeftLeg3(int i) { return (isLowerLeftLeg3(i) || i == 10 || i == 27); }
bool isLowerLeftLeg3(int i) { return (i == 11 || i == 22); }
bool isLeftLeg4(int i) { return (isLowerLeftLeg4(i) || i == 18 || i == 28); }
bool isLowerLeftLeg4(int i) { return (i == 19 || i == 29); }

bool isRightLeg1(int i) { return (isLowerRightLeg1(i) || i == 14 || i == 34); }
bool isLowerRightLeg1(int i) { return (i == 15 || i == 35); }
bool isRightLeg2(int i) { return (isLowerRightLeg2(i) || i == 13 || i == 33); }
bool isLowerRightLeg2(int i) { return (i == 12 || i == 36); }
bool isRightLeg3(int i) { return (isLowerRightLeg3(i) || i == 17 || i == 32); }
bool isLowerRightLeg3(int i) { return (i == 16 || i == 37); }
bool isRightLeg4(int i) { return (isLowerRightLeg4(i) || i == 21 || i == 31); }
bool isLowerRightLeg4(int i) { return (i == 20 || i == 30); }

bool isLeg(int i)
{
	return (isLeftLeg1(i) || isLeftLeg2(i) || isLeftLeg3(i) || isLeftLeg4(i) ||
	       isRightLeg1(i) || isRightLeg2(i) || isRightLeg3(i) || isRightLeg4(i));
}

bool isLowerLeg(int i)
{
	return (isLowerLeftLeg1(i) || isLowerLeftLeg2(i) || isLowerLeftLeg3(i) || isLowerLeftLeg4(i) ||
	       isLowerRightLeg1(i) || isLowerRightLeg2(i) || isLowerRightLeg3(i) || isLowerRightLeg4(i));
}

// Calcualtes the shift to be centered at the origin
float calculateShift(float minCoord, float maxCoord)
{
	float coordExtent;
	coordExtent = maxCoord - minCoord;
	return minCoord + (coordExtent/2.0f);
}

void getShiftAndScale(vec3 *shift, vec3 *scale, vec3 *gMin, vec3 *gMax)
{
	float maxExtent, xExtent, yExtent, zExtent;
	xExtent = gMax->x-gMin->x;
	yExtent = gMax->y-gMin->y;
	zExtent = gMax->z-gMin->z;
	if (xExtent >= yExtent && xExtent >= zExtent)
	{
		maxExtent = xExtent;
	}
	if (yExtent >= xExtent && yExtent >= zExtent)
	{
		maxExtent = yExtent;
	}
	if (zExtent >= xExtent && zExtent >= yExtent)
	{
		maxExtent = zExtent;
	}
	scale->x = 2.0f / maxExtent;
	shift->x = gMin->x + (xExtent / 2.0f);
	scale->y = 2.0f / maxExtent;
	shift->y = gMin->y + (yExtent / 2.0f);
	scale->z = 2.0f / maxExtent;
	shift->z = gMin->z + (zExtent / 2.0f);
}

// pivot around given part
void setupPart(const shared_ptr<vector<shared_ptr<Shape>>> shapes,
			   shared_ptr<MatrixStack> Model, int i, int pivot, vec3 *angle)
{
	float x = calculateShift(shapes->at(pivot)->min.x, shapes->at(pivot)->max.x);
	float y = calculateShift(shapes->at(pivot)->min.y, shapes->at(pivot)->max.y);
	float z = calculateShift(shapes->at(pivot)->min.z, shapes->at(pivot)->max.z);
	Model->pushMatrix();
		Model->translate(vec3(x, y, z));
		Model->rotate(angle->z, vec3(0, 0, 1));
		Model->rotate(angle->y, vec3(0, 1, 0));
		Model->rotate(angle->x, vec3(1, 0, 0));
		Model->translate(vec3(-x, -y, -z));
}

// draws body part (i) and rotates body part around angle at given pivot 
void drawPart(const shared_ptr<Program> prog, 
			  const shared_ptr<vector<shared_ptr<Shape>>> shapes,
			  shared_ptr<MatrixStack> Model, int i, int pivot, vec3 *angle)
{
	setupPart(shapes, Model, i, pivot, angle);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		shapes->at(i)->draw(prog);
	Model->popMatrix();
}

// draws leg rotated around waste, lower leg rotated around knee
void drawLeg(const shared_ptr<Program> prog, 
			 const shared_ptr<vector<shared_ptr<Shape>>> shapes,
			 shared_ptr<MatrixStack> Model, int i,
			 int wastePiv, vec3 *wasteAng, int kneePiv, vec3 *kneeAng)
{
	setupPart(shapes, Model, i, wastePiv, wasteAng);
	if (isLowerLeg(i))
	{
		setupPart(shapes, Model, i, kneePiv, kneeAng);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		shapes->at(i)->draw(prog);
		Model->popMatrix();
	}
	else
	{
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		shapes->at(i)->draw(prog);
	}
	Model->popMatrix();
}

// account for precision errors for float comparisons
bool floatEqual(float a, float b)
{
	float delta = 0.0001;
	return (a > b-delta && a < b+delta);
}

// Increments (numIncrements amount of times) until primary is at next if cond is true
void animatePart(float *primary, float previous, float next, bool cond, float numIncrements)
{
	//numIncrements = numIncrements/1.3;
	//numIncrements = numIncrements/dummySpeed;
	if (next > previous) // increment
	{
		if (*primary > next)
			*primary = next;
		else if (cond && !(floatEqual(*primary, next)))
			*primary += abs(previous-next)/numIncrements;
	}
	else // decrement
	{
		if (*primary < next)
			*primary = next;
		else if (cond && !(floatEqual(*primary, next)))
			*primary -= abs(previous-next)/numIncrements;
	}
}
