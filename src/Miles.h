#ifndef _DUMMY_H_
#define _DUMMY_H_

#include <vector>

class Program;
class Shape;
class MatrixStack;

float calculateShift(float minCoord, float maxCoord);
void getShiftAndScale(glm::vec3 *shift, glm::vec3 *scale, glm::vec3 *gMin, glm::vec3 *gMax);

bool isLeftLeg1(int i);
bool isLowerLeftLeg1(int i);
bool isLeftLeg2(int i);
bool isLowerLeftLeg2(int i);
bool isLeftLeg3(int i);
bool isLowerLeftLeg3(int i);
bool isLeftLeg4(int i);
bool isLowerLeftLeg4(int i);

bool isRightLeg1(int i);
bool isLowerRightLeg1(int i);
bool isRightLeg2(int i);
bool isLowerRightLeg2(int i);
bool isRightLeg3(int i);
bool isLowerRightLeg3(int i);
bool isRightLeg4(int i);
bool isLowerRightLeg4(int i);

bool isLeg(int i);
bool isLowerLeg(int i);

void animatePart(float *primary, float previous, float next, bool cond, float numIncrements);
bool floatEqual(float a, float b);

void setupPart(const std::shared_ptr<std::vector<std::shared_ptr<Shape>>> shape,
			   std::shared_ptr<MatrixStack> Model, int i, int pivot, glm::vec3 *angle);
void drawPart(const std::shared_ptr<Program> prog, 
			  const std::shared_ptr<std::vector<std::shared_ptr<Shape>>> shapes,
			  std::shared_ptr<MatrixStack> Model, int i, int pivot, glm::vec3 *angle);
void drawLeg(const std::shared_ptr<Program> prog, 
		     const std::shared_ptr<std::vector<std::shared_ptr<Shape>>> shapes,
			 std::shared_ptr<MatrixStack> Model, int i, 
			 int wastePiv, glm::vec3 *wasteAng, int kneePiv, glm::vec3 *kneeAng);

#endif
