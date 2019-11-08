#ifndef _MILES_H_
#define _MILES_H_

#include <vector>

class Program;
class Shape;
class MatrixStack;

struct animVar {
	float cur;
	float prev;
};

struct animVec {
	animVar x;
	animVar y;
	animVar z;
};

void initAnimVec(animVec *v, float x, float y, float z);

class Miles
{
	public:
		Miles();
		virtual ~Miles();
		float getTranslateX();
		float getTranslateZ();
		void initialize(std::shared_ptr<std::vector<std::shared_ptr<Shape>>> shapes, glm::vec3 *gMin, glm::vec3 *gMax);
		void initializeNext(std::shared_ptr<std::vector<std::shared_ptr<Shape>>> shapes, glm::vec3 *gMin, glm::vec3 *gMax);
		void initStartingPosition();
		void updatePosition();
		void updateAnimVars();
		void updateAnimVec(animVec *v);
		void draw(const std::shared_ptr<Program> prog);

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

		void animatePart(animVar *primary, float next, bool cond, float numIncrements);
		bool floatEqual(float a, float b);

		void setupPart(const std::shared_ptr<std::vector<std::shared_ptr<Shape>>> shape,
					   std::shared_ptr<MatrixStack> Model, int i, int pivot, glm::vec3 *angle);
		void setupPartWithScale(const std::shared_ptr<std::vector<std::shared_ptr<Shape>>> shape,
					   std::shared_ptr<MatrixStack> Model, int i, int pivot, glm::vec3 *angle, glm::vec3 *scale);
		void drawLeg(const std::shared_ptr<Program> prog, 
				     const std::shared_ptr<std::vector<std::shared_ptr<Shape>>> shapes,
					 std::shared_ptr<MatrixStack> Model, int i, 
					 int wastePiv, glm::vec3 *wasteAng, int kneePiv, glm::vec3 *kneeAng);

	private:
		std::shared_ptr<std::vector<std::shared_ptr<Shape>>> shapes;
		std::shared_ptr<std::vector<std::shared_ptr<Shape>>> nextShapes;
		int positionCount;
		animVec translate;
		animVar tempCount;
		animVar headSideToSide;
		animVar bodySideToSide;
		animVar pauses;
		animVec rotateMiles;
		animVec rotateHead;
		float shoulderRotate;
		float armRotate;
		animVar legRotate;
		animVec L1K;
		animVec L1S;
		animVec R1K;
		animVec R1S;
		animVec eyeScale;

		glm::vec3 shift;
		glm::vec3 scale;
		glm::vec3 nextShift;
		glm::vec3 nextScale;
};


float calculateShift(float minCoord, float maxCoord);
void getShiftAndScale(glm::vec3 *shift, glm::vec3 *scale, glm::vec3 *gMin, glm::vec3 *gMax);
#endif