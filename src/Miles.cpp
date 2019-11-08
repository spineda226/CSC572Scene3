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

Miles::Miles()
{
}

Miles::~Miles()
{
}

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

void Miles::initialize(shared_ptr<vector<shared_ptr<Shape>>> shapes, vec3 *gMin, vec3 *gMax)
{
	this->shapes = shapes;
	getShiftAndScale(&this->shift, &this->scale, gMin, gMax);
}

void Miles::initializeNext(shared_ptr<vector<shared_ptr<Shape>>> shapes, vec3 *gMin, vec3 *gMax)
{
	this->nextShapes = shapes;
	getShiftAndScale(&this->nextShift, &this->nextScale, gMin, gMax);
}

void Miles::updateAnimVec(animVec *v)
{
	v->x.prev = v->x.cur;
	v->y.prev = v->y.cur;
	v->z.prev = v->z.cur;
}

void Miles::updateAnimVars()
{
	updateAnimVec(&translate);
	updateAnimVec(&rotateMiles);
	updateAnimVec(&rotateHead);
	updateAnimVec(&L1K);
	updateAnimVec(&L1S);
	updateAnimVec(&R1K);
	updateAnimVec(&R1S);
	updateAnimVec(&eyeScale);
	tempCount.prev = tempCount.cur;
	legRotate.prev = legRotate.cur;
	pauses.prev = pauses.cur;
	headSideToSide.prev = headSideToSide.cur;
	bodySideToSide.prev = bodySideToSide.cur;
}

void Miles::initStartingPosition()
{
	initAnimVec(&translate, -4, -0.4, -4.1);
	initAnimVec(&rotateHead, 0, 0, PI/4);
	initAnimVec(&eyeScale, 1, 1, 1);
	positionCount = 5;
	tempCount = {0};
	headSideToSide = {0};
	pauses = {0};
	bodySideToSide = {0};
	initAnimVec(&rotateMiles, 0.6*PI, PI/2, 0);
	shoulderRotate = 0;
	armRotate = 0;
	legRotate = {0};
	initAnimVec(&L1K, 0, 0, 0);
	initAnimVec(&L1S, 0, 0, 0);
	initAnimVec(&R1S, 0, 0, 0);
	initAnimVec(&R1K, 0, 0, 0);
}

bool Miles::isLeftLeg1(int i) { return (isLowerLeftLeg1(i) || i == 6 || i == 25); }
bool Miles::isLowerLeftLeg1(int i) { return (i == 7 || i == 24); }
bool Miles::isLeftLeg2(int i) { return (isLowerLeftLeg2(i) || i == 8 || i == 26); }
bool Miles::isLowerLeftLeg2(int i) { return (i == 9 || i == 23); }
bool Miles::isLeftLeg3(int i) { return (isLowerLeftLeg3(i) || i == 10 || i == 27); }
bool Miles::isLowerLeftLeg3(int i) { return (i == 11 || i == 22); }
bool Miles::isLeftLeg4(int i) { return (isLowerLeftLeg4(i) || i == 18 || i == 28); }
bool Miles::isLowerLeftLeg4(int i) { return (i == 19 || i == 29); }

bool Miles::isRightLeg1(int i) { return (isLowerRightLeg1(i) || i == 14 || i == 34); }
bool Miles::isLowerRightLeg1(int i) { return (i == 15 || i == 35); }
bool Miles::isRightLeg2(int i) { return (isLowerRightLeg2(i) || i == 13 || i == 33); }
bool Miles::isLowerRightLeg2(int i) { return (i == 12 || i == 36); }
bool Miles::isRightLeg3(int i) { return (isLowerRightLeg3(i) || i == 17 || i == 32); }
bool Miles::isLowerRightLeg3(int i) { return (i == 16 || i == 37); }
bool Miles::isRightLeg4(int i) { return (isLowerRightLeg4(i) || i == 21 || i == 31); }
bool Miles::isLowerRightLeg4(int i) { return (i == 20 || i == 30); }

bool Miles::isLeg(int i)
{
	return (isLeftLeg1(i) || isLeftLeg2(i) || isLeftLeg3(i) || isLeftLeg4(i) ||
	       isRightLeg1(i) || isRightLeg2(i) || isRightLeg3(i) || isRightLeg4(i));
}

void initAnimVec(animVec *v, float x, float y, float z)
{
	v->x = {x, x};
	v->y = {y, y};
	v->z = {z, z};
}

bool Miles::isLowerLeg(int i)
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
void Miles::setupPart(const shared_ptr<vector<shared_ptr<Shape>>> shapes,
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

// scale a piece and pivot around given part 
void Miles::setupPartWithScale(const shared_ptr<vector<shared_ptr<Shape>>> shapes,
			   shared_ptr<MatrixStack> Model, int i, int pivot, vec3 *angle, vec3 *scale)
{
	float x = calculateShift(shapes->at(pivot)->min.x, shapes->at(pivot)->max.x);
	float y = calculateShift(shapes->at(pivot)->min.y, shapes->at(pivot)->max.y);
	float z = calculateShift(shapes->at(pivot)->min.z, shapes->at(pivot)->max.z);

	float x_i = calculateShift(shapes->at(i)->min.x, shapes->at(i)->max.x);
	float y_i = calculateShift(shapes->at(i)->min.y, shapes->at(i)->max.y);
	float z_i = calculateShift(shapes->at(i)->min.z, shapes->at(i)->max.z);
	Model->pushMatrix();
		Model->translate(vec3(x, y, z));
		Model->rotate(angle->z, vec3(0, 0, 1));
		Model->rotate(angle->y, vec3(0, 1, 0));
		Model->rotate(angle->x, vec3(1, 0, 0));
		Model->translate(vec3(-x, -y, -z));
		Model->translate(vec3(x_i, y_i, z_i));
		Model->scale(*scale);
		Model->translate(vec3(-x_i, -y_i, -z_i));
}

// draws leg rotated around waste, lower leg rotated around knee
void Miles::drawLeg(const shared_ptr<Program> prog, 
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
bool Miles::floatEqual(float a, float b)
{
	float delta = 0.0001;
	return (a > b-delta && a < b+delta);
}

// Increments (numIncrements amount of times) until primary is at next if cond is true
void Miles::animatePart(animVar *primary, float next, bool cond, float numIncrements)
{
	if (next > 0) // increment
	{
		if (primary->cur > primary->prev + next)
			primary->cur = primary->prev + next;
		else if (cond && !(floatEqual(primary->cur, primary->prev + next)))
			primary->cur += next/numIncrements;
	}
	else // decrement
	{
		if (primary->cur < primary->prev + next)
			primary->cur = primary->prev + next;
		else if (cond && !(floatEqual(primary->cur, primary->prev + next)))
			primary->cur -= abs(next)/numIncrements;
	}
}

float Miles::getTranslateX() { return translate.x.cur; };
float Miles::getTranslateZ() { return translate.z.cur; };

void Miles::updatePosition()
{
	/*
	if (positionCount == 1) // Roll Over
	{
		animatePart(&rotateMiles.y, -6*PI, true, 60);
		animatePart(&translate.x, 4, true, 60);
		if (floatEqual(translate.x.cur, translate.x.prev+4))
		{
			positionCount++;
			updateAnimVars();
		}
	}
	else if (positionCount == 2) // Move legs while upside down
	{
		animatePart(&tempCount, 10, true, 100);
		legRotate = tempCount;
		rotateMiles.y.cur = (rotateMiles.y.prev + cos(tempCount.cur)/3);
		if (floatEqual(tempCount.cur, tempCount.prev+10))
		{
			positionCount++;
			updateAnimVars();
		}
	}
	else if (positionCount == 3) // Flip up and look left
	{

		animatePart(&rotateMiles.y, PI-cos(tempCount.prev)/3, true, 20);
		animatePart(&rotateHead.z, PI/4, rotateMiles.y.cur > rotateMiles.y.prev + PI/2, 90);
		
		animatePart(&pauses, 10, rotateHead.z.cur > rotateHead.z.prev + PI/4.1, 100);
		animatePart(&rotateHead.y, -PI/4, pauses.cur > pauses.prev + 9, 50);
		animatePart(&tempCount, -5, pauses.cur > pauses.prev + 9, 50);
		legRotate.cur = tempCount.cur;
		animatePart(&rotateMiles.z, PI/4, pauses.cur > pauses.prev + 8, 50);
		if (floatEqual(tempCount.cur, tempCount.prev - 5))
		{
			positionCount++;
			updateAnimVars();
		}
		
	}
	else if (positionCount == 4) // look right
	{
		animatePart(&pauses, -10, true, 60);
		animatePart(&rotateHead.y, 3*PI/8, pauses.cur < pauses.prev - 8, 50);
		animatePart(&tempCount, -5, pauses.cur < pauses.prev - 8, 50);
		legRotate = tempCount;
		animatePart(&rotateMiles.z, -3*PI/8, pauses.cur < pauses.prev - 8, 50);
		if (floatEqual(tempCount.cur, tempCount.prev - 5))
		{
			positionCount++;
			updateAnimVars();
		}
	}
	else if (positionCount == 5) // move to trash and get hat
	{
		animatePart(&pauses, 10, true, 60);
		animatePart(&translate.x, 3, pauses.cur > pauses.prev + 8, 50);
		animatePart(&tempCount, 7, pauses.cur > pauses.prev + 7, 50);
		legRotate = tempCount;
		if (floatEqual(tempCount.cur, tempCount.prev + 7))
		{
			positionCount++;
			this->shapes = this->nextShapes;
			this->shift = this->nextShift;
			tempCount = {0, 0};
			legRotate = {0, 0};
			translate.x.cur = 3;
			armRotate = 0;
			shoulderRotate = 0;
			initAnimVec(&rotateHead, 0, 0, 0);
			initAnimVec(&rotateMiles, 0, 5*PI/4, 0);
			updateAnimVars();
		}
	}
	*/
	if (positionCount == 5) // move to trash and get hat
	{
		positionCount++;
		this->shapes = this->nextShapes;
		this->shift = this->nextShift;
		tempCount = {0, 0};
		legRotate = {0, 0};
		translate.x.cur = 3;
		armRotate = 0;
		shoulderRotate = 0;
		initAnimVec(&rotateHead, 0, 0, 0);
		initAnimVec(&rotateMiles, 0, 5*PI/4, 0);
		updateAnimVars();
	}
	else if (positionCount == 6) // walk out on two legs
	{
		animatePart(&pauses, -10, true, 400);
		animatePart(&tempCount, 20, pauses.cur < pauses.prev - 5, 200);
		legRotate = tempCount;
		animatePart(&translate.x, -3, pauses.cur < pauses.prev - 5, 200);
		animatePart(&translate.z, 1, pauses.cur < pauses.prev - 5, 200);
		animatePart(&rotateMiles.y, 9*PI/4, translate.x.cur < translate.x.prev - 2, 50);
		if (floatEqual(translate.x.cur, translate.x.prev - 3))
		{
			positionCount++;
			updateAnimVars();
		}
	}
	else if (positionCount == 7) // Put arm down and start spiderman dance
	{
		animatePart(&pauses, 10, true, 50);
		animatePart(&L1K.z, -1.6, pauses.cur > pauses.prev + 8, 20);
		
		animatePart(&L1S.x, -0.8, pauses.cur > pauses.prev + 8, 20);
		animatePart(&L1S.y, -0.8, pauses.cur > pauses.prev + 8, 20);
		animatePart(&tempCount, -10, pauses.cur > pauses.prev + 8, 100);
		animatePart(&headSideToSide, -1.6, pauses.cur > pauses.prev + 8, 50);
		animatePart(&bodySideToSide, 10, pauses.cur > pauses.prev + 8, 100);
		if (floatEqual(tempCount.cur, tempCount.prev - 10))
		{
			positionCount++;
			updateAnimVars();
		}
	}
	else if (positionCount == 8) // look the other way
	{
		animatePart(&headSideToSide, -3, true, 50);
		animatePart(&tempCount, -10, true, 100);
		animatePart(&bodySideToSide, -10, true, 100);
		if (floatEqual(tempCount.cur, tempCount.prev -10))
		{
			positionCount++;
			updateAnimVars();
		}
	}
	else if (positionCount == 9) // put hands up
	{
		animatePart(&headSideToSide, 3, true, 50);
		animatePart(&bodySideToSide, 10, true, 50);
		animatePart(&L1K.z, 1.6, true, 50);
		animatePart(&L1S.x, 0.8, true, 50);
		animatePart(&L1S.y, 0.8, true, 50);
		animatePart(&R1K.y, -1, true, 50);
		animatePart(&R1K.z, 1, true, 50);
		animatePart(&R1S.z, 1, true, 50);
		if (floatEqual(R1S.z.cur, R1S.z.prev + 1))
		{
			positionCount++;
			updateAnimVars();
		}
	}
	else if (positionCount == 10) // put hands down make eyes big
	{
		animatePart(&pauses, 10, true, 200);
		animatePart(&headSideToSide, -1.5, pauses.cur > pauses.prev + 2.5, 50);
		animatePart(&L1K.z, -1.6, pauses.cur > pauses.prev + 2.5, 50);
		animatePart(&L1S.x, -0.8, pauses.cur > pauses.prev + 2.5, 50);
		animatePart(&L1S.y, -0.8, pauses.cur > pauses.prev + 2.5, 50);
		animatePart(&R1K.y, 1, pauses.cur > pauses.prev + 2.5, 50);
		animatePart(&R1K.z, -1, pauses.cur > pauses.prev + 2.5, 50);
		animatePart(&R1S.z, -1, pauses.cur > pauses.prev + 2.5, 50);
		animatePart(&eyeScale.x, 0.5, pauses.cur > pauses.prev + 7.5, 5);
		animatePart(&eyeScale.y, 0.5, pauses.cur > pauses.prev + 7.5, 5);
		animatePart(&eyeScale.z, 0.5, pauses.cur > pauses.prev + 7.5, 5);
		if (floatEqual(pauses.cur, pauses.prev + 10))
		{
			positionCount++;
			updateAnimVars();
		}
	}
	else if (positionCount == 11) // run toward portal
	{
		animatePart(&tempCount, -25, true, 100);
		legRotate = tempCount;
		animatePart(&translate.z, 3.1, true, 25);
	}

}

void Miles::draw(const std::shared_ptr<Program> prog)
{
	// Draw miles
	auto Model = make_shared<MatrixStack>();
	Model->pushMatrix();
		Model->loadIdentity();
		Model->translate(vec3(translate.x.cur, translate.y.cur, translate.z.cur));
		Model->rotate(rotateMiles.x.cur, vec3(1, 0, 0)); // axis left and right through body
		Model->rotate(rotateMiles.z.cur, vec3(0, 0, 1));
		Model->rotate(rotateMiles.y.cur, vec3(0, 1, 0)); // axis up and down through head
		Model->scale(vec3(0.6)*scale);
		Model->translate(vec3(-1,-1,-1)*shift);
		SetMaterial(prog, 0);		
		int shapesSize = shapes->size();
		armRotate = sin(tempCount.cur);
		shoulderRotate = cos(tempCount.cur)/2;
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
				vec3 bodyRotate = vec3(-0.2*abs(sin(bodySideToSide.cur)), 0, 0);
				setupPart(shapes, Model, i, 0, &bodyRotate); 
			}
			if ((i >= 1 && i <= 5) || i == 38) // Head
			{
				vec3 headRotate = vec3(rotateHead.x.cur + sin(headSideToSide.cur)/4, rotateHead.y.cur + PI*sin(headSideToSide.cur)/4, rotateHead.z.cur);
				if (i == 2 || i == 4) // eyes
				{
					vec3 eye_sc = vec3(eyeScale.x.cur, eyeScale.y.cur, eyeScale.z.cur);
					setupPartWithScale(shapes, Model, i, 1, &headRotate, &eye_sc);
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
					shapes->at(i)->draw(prog);	
				}
				else
				{
					setupPart(shapes, Model, i, 1, &headRotate);
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
					shapes->at(i)->draw(prog);	
				}
				Model->popMatrix();
			}
			else if (isLeftLeg1(i))
			{
				vec3 rotateL1S = vec3(shoulderRotate + L1S.x.cur, -shoulderRotate + L1S.y.cur, -shoulderRotate+L1S.z.cur);
				vec3 rotateL1K = vec3(L1K.x.cur, L1K.y.cur, L1K.z.cur);
				drawLeg(prog, shapes, Model, i, 25, &rotateL1S, 24, &rotateL1K);
			}
			else if (isLeftLeg2(i))
			{
				vec3 rotateL2S = vec3(0, -shoulderRotate/2, 0);
				vec3 rotateL2K = vec3(armRotate, 0, armRotate);
				drawLeg(prog, shapes, Model, i, 26, &rotateL2S, 23, &rotateL2K);
			}
			else if (isLeftLeg3(i))
			{
				vec3 rotateL3S = vec3(0, 0, shoulderRotate);
				vec3 rotateL3K = vec3(armRotate, -armRotate, 0);
				drawLeg(prog, shapes, Model, i, 27, &rotateL3S, 22, &rotateL3K);
			}
			else if (isLeftLeg4(i))
			{
				vec3 rotateL4S = vec3(0, 0, sin(legRotate.cur)/4 - 0.4);
				vec3 rotateL4K = vec3(0, 0, sin(legRotate.cur)/6 + 0.5);
				drawLeg(prog, shapes, Model, i, 28, &rotateL4S, 29, &rotateL4K);
			}
			else if (isRightLeg1(i))
			{
				vec3 rotateR1S = vec3(R1S.x.cur, R1S.y.cur+shoulderRotate/2, R1S.z.cur);
				vec3 rotateR1K = vec3(R1K.x.cur+-armRotate/2, R1K.y.cur, R1K.z.cur);
				drawLeg(prog, shapes, Model, i, 34, &rotateR1S, 35, &rotateR1K);
			}
			else if (isRightLeg2(i))
			{
				vec3 rotateR2S = vec3(0);
				vec3 rotateR2K = vec3(-armRotate, 0, 0);
				drawLeg(prog, shapes, Model, i, 33, &rotateR2S, 36, &rotateR2K);
			}
			else if (isRightLeg3(i))
			{
				vec3 rotateR3S = vec3(0, 0, shoulderRotate);
				vec3 rotateR3K = vec3(-armRotate, 0, 0);
				drawLeg(prog, shapes, Model, i, 32, &rotateR3S, 37, &rotateR3K);
			}
			else if (isRightLeg4(i))
			{
				vec3 rotateR4S = vec3(0, 0, sin(-legRotate.cur)/4 - 0.4);
				vec3 rotateR4K = vec3(0, 0, sin(-legRotate.cur)/6 + 0.5);
				drawLeg(prog, shapes, Model, i, 31, &rotateR4S, 30, &rotateR4K);
			}
			else
			{
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
				shapes->at(i)->draw(prog);	
			}
			if (!(isLeftLeg4(i) || isRightLeg4(i)))
			{
				Model->popMatrix();
			}
		}
	Model->popMatrix();
}